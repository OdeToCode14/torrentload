#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <thread>
#include <bits/stdc++.h>
#include <iostream>
#include <mutex> 

#define pb push_back
#define mk make_pair
using namespace std;

mutex output_device_mtx;
mutex seeders_mtx;

string my_address;
string other_tracker_address;
string seeder_list_file_path;
string log_file_path;

//map<string,vector<pair<string,string>>> seeders;
map<string,map<string,string>> seeders;

int get_other_tracker_connection();

void print_on_screen(string str){
   lock_guard<mutex> guard(output_device_mtx);
        cout<<str<<"\n";
    //mtx.unlock();
}
string get_ip_address(string address){
    int ind=address.find(":");
    string ip_address=address.substr(0,ind);
    return ip_address;
}
string get_port_address(string address){
    int ind=address.find(":");
    string ip_address=address.substr(ind+1);
    return ip_address;
}

void send_message(int socket_id,string message){
    send(socket_id, message.c_str(), message.length()+1, 0);
}

string receive_message(int socket_id){
    char buffer[256];
    int BUFFER_SIZE=256;
    int length = recv(socket_id, buffer, BUFFER_SIZE, 0);
    buffer[length]='\0';
    string message=(string) buffer;
    //cout<<"received "<<length<<" "<<message<<"\n";
    return message;
}

void access_seeders(string operation,string file_name,string file_hash,string address,int client_socket_id){ // ciritical section
    print_on_screen(operation);
    lock_guard<mutex> guard(seeders_mtx);
    if(operation == "share"){
            if(seeders.find(file_hash) != seeders.end()){
                seeders[file_hash][address]=file_name;
            }
            else{
                seeders[file_hash]=map<string,string>();
                seeders[file_hash][address]=file_name;
                //print_on_screen("added");
            }
    }
    else if(operation == "get"){
            if(seeders.find(file_hash) != seeders.end()){
                string seeder_list="";
                bool flag=false;
                for(auto it : seeders[file_hash]){
                    seeder_list=seeder_list + it.first +" ";
                    flag=true;
                }
                if(flag == false){
                    send_message(client_socket_id,"not found");
                }
                else{
                    send_message(client_socket_id,seeder_list);
                }
            }
            else{
                send_message(client_socket_id,"not found");
            }
    }
    else if(operation == "remove"){
            if(seeders.find(file_hash) != seeders.end()){
                if(seeders[file_hash].find(address) != seeders[file_hash].end()){
                    seeders[file_hash].erase(address);
                    send_message(client_socket_id,"removed");
                }
                else{
                    send_message(client_socket_id,"you are not in seeder list of this file");
                }
            
            }
            else{
                send_message(client_socket_id,"file not found");
            }
    }

}
void serve(int client_socket_id){
    //cout<<"here\n";
    //cout<<client_socket_id<<"\n";
    
    string message=receive_message(client_socket_id);
    print_on_screen(message);
    if(message == "share"){
        string response="ok";
        send_message(client_socket_id,response);
        string file_name=receive_message(client_socket_id);
        send_message(client_socket_id,response);
        string file_hash=receive_message(client_socket_id); // this is hash of hash
        send_message(client_socket_id,response);
        string address=receive_message(client_socket_id);
        send_message(client_socket_id,response);
        
        access_seeders(message,file_name,file_hash,address,client_socket_id);


        int other_tracker_socket_id=get_other_tracker_connection();
        if(other_tracker_socket_id != -1){
            send_message(other_tracker_socket_id,"share from tracker");
            string reply=receive_message(other_tracker_socket_id);
            send_message(other_tracker_socket_id,file_name);
            reply=receive_message(other_tracker_socket_id);
            send_message(other_tracker_socket_id,file_hash);
            reply=receive_message(other_tracker_socket_id);
            send_message(other_tracker_socket_id,address);
            reply=receive_message(other_tracker_socket_id);
            close(other_tracker_socket_id);
        }

        //cout<<file_name <<" "<< file_hash << " "<<address<<"\n";
        
        /*if(seeders.find(file_hash) != seeders.end()){
            seeders[file_hash][address]=file_name;
        }
        else{
            seeders[file_hash]=map<string,string>();
            seeders[file_hash][address]=file_name;
        }*/
        //cout<<"this one "<<seeder[file_hash][address]<<"\n";

    }
    else if(message == "get"){
        string response="ok";
        //print_on_screen("got you dude");
        send_message(client_socket_id,response);
        string file_hash=receive_message(client_socket_id); // this is hash of hash
        //send_message(client_socket_id,response);
        
        //cout<<file_name <<" "<< file_hash << " "<<address<<"\n";
        
        access_seeders(message,"",file_hash,"",client_socket_id);
        /*if(seeders.find(file_hash) != seeders.end()){
            string seeder_list="";
            bool flag=false;
            for(auto it : seeders[file_hash]){
                seeder_list=seeder_list + it.first +" ";
                flag=true;
            }
            if(flag == false){
                send_message(client_socket_id,"not found");
            }
            else{
                send_message(client_socket_id,seeder_list);
            }
        }
        else{
            send_message(client_socket_id,"not found");
        }*/
        
    }
    else if(message == "remove"){
        string response="ok";
        send_message(client_socket_id,response);
        string file_hash=receive_message(client_socket_id); // this is hash of hash
        send_message(client_socket_id,response);
        string address=receive_message(client_socket_id);
        //send_message(client_socket_id,response);
        
        //cout<<file_name <<" "<< file_hash << " "<<address<<"\n";
        access_seeders(message,"",file_hash,address,client_socket_id);

        int other_tracker_socket_id=get_other_tracker_connection();
        if(other_tracker_socket_id != -1){
            send_message(other_tracker_socket_id,"remove from tracker");
            string reply=receive_message(other_tracker_socket_id);
            send_message(other_tracker_socket_id,file_hash);
            reply=receive_message(other_tracker_socket_id);
            send_message(other_tracker_socket_id,address);
            close(other_tracker_socket_id);
        }
        /*if(seeders.find(file_hash) != seeders.end()){
            if(seeders[file_hash].find(address) != seeders[file_hash].end()){
                seeders[file_hash].erase(address);
                send_message(client_socket_id,"removed");
            }
            else{
                send_message(client_socket_id,"you are not in seeder list of this file");
            }
            
        }
        else{
            send_message(client_socket_id,"file not found");
        }*/   
    }
    else if(message == "synchronize"){
        string response="ok";
        print_on_screen("got your request");
        send_message(client_socket_id,response);
        string rand=receive_message(client_socket_id);

        ofstream file (seeder_list_file_path);
        if (file.is_open()){
            //myfile << "This is a line.\n";
            //myfile << "This is another line.\n";
            for(auto mp : seeders){
                string hash=mp.first;
                string file_name="";
                string seeder_list="";
                for(auto it : mp.second){
                    seeder_list=seeder_list + it.first + " ";
                    file_name=it.second;
                }
                if(file_name == ""){  //empty seeder list need not add
                    continue;
                }
                string list_enry=hash+"-"+file_name+"-"+seeder_list;
                file << list_enry;
                file<<"\n";
                //send_message(client_socket_id,list);
                //cout<<"sending "<<list<<"\n";
                //print_on_screen("sending "+list);

                //receive_message(client_socket_id);
            }
            file.close();
            print_on_screen("file prepared");
            char buffer[256];
            int BUFFER_SIZE=256;
            FILE *fd = fopen(seeder_list_file_path.c_str(), "rb");
            size_t rret, wret;
            int bytes_read;
            while (!feof(fd)) {
                if ((bytes_read = fread(&buffer, 1, BUFFER_SIZE, fd)) > 0){
                    send(client_socket_id, buffer, bytes_read, 0);
                    print_on_screen("sent "+to_string(bytes_read));
                }
                else
                    break;
            }
            fclose(fd);
        }
    }
    else if(message == "share from tracker"){
        string response="ok";
        send_message(client_socket_id,response);
        string file_name=receive_message(client_socket_id);
        send_message(client_socket_id,response);
        string file_hash=receive_message(client_socket_id); // this is hash of hash
        send_message(client_socket_id,response);
        string address=receive_message(client_socket_id);
        send_message(client_socket_id,response);
        
        access_seeders("share",file_name,file_hash,address,client_socket_id);
    }
    else if(message == "remove from tracker"){
        string response="ok";
        send_message(client_socket_id,response);
        string file_hash=receive_message(client_socket_id); // this is hash of hash
        send_message(client_socket_id,response);
        string address=receive_message(client_socket_id);
        //send_message(client_socket_id,response);
        
        //cout<<file_name <<" "<< file_hash << " "<<address<<"\n";
        access_seeders("remove","",file_hash,address,client_socket_id);
    }
    //cout<<"processed\n";
    print_on_screen("processed");
    close(client_socket_id);
    print_on_screen("closed connection");
}

int get_other_tracker_connection(){
    string other_tracker_ip=get_ip_address(other_tracker_address);
    int other_tracker_port=stoi(get_port_address(other_tracker_address));

    int other_tracker_socket_id;
    int len;
    struct sockaddr_in address;
    int result;
    other_tracker_socket_id = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(other_tracker_ip.c_str()); //add tracker address here
    address.sin_port = other_tracker_port;
    len = sizeof(address);

    result = connect(other_tracker_socket_id, (struct sockaddr *)&address, len);
    if(result == -1) {
        return -1;
    }
    return other_tracker_socket_id;
}

vector<string> split_string(string command){  // improve for multiple spaces
      char delimiter=' ';
      vector<string> processed_command;
      stringstream ss(command);
      string token;
     
      while(getline(ss, token, delimiter)) {
        processed_command.push_back(token);
      }
      return processed_command;
}

void synchronize_with_other_tracker(){
    int other_tracker_socket_id=get_other_tracker_connection();
    if(other_tracker_socket_id == -1){
        //print_on_screen("Other tracker is down");
        return;
    }
    send_message(other_tracker_socket_id,"synchronize");
    string response=receive_message(other_tracker_socket_id);
    print_on_screen(response);
    send_message(other_tracker_socket_id,"ok");
    //print_on_screen(receive_message(other_tracker_socket_id));
    //lock_guard<mutex> guard(seeder_list_file_mtx);
    size_t datasize;
    FILE* fd = fopen(seeder_list_file_path.c_str(), "wb");
    char buffer[256];
    int BUFFER_SIZE=256;
    while (true){
        datasize = recv(other_tracker_socket_id, buffer, BUFFER_SIZE, 0);
        if(datasize == 0){
            break;
        }
        fwrite(&buffer, 1, datasize, fd);
        //print_on_screen(to_string(datasize));
    }
    fclose(fd);
    close(other_tracker_socket_id);

    string line;
    ifstream in (seeder_list_file_path);
    if (in.is_open()){
        while ( getline (in,line) ){
            int first=line.find("-");
            string file_hash=line.substr(0,first);
            line=line.substr(first+1);
            int second=line.find("-");
            string file_name=line.substr(0,second);
            line=line.substr(second+1);
            //print_on_screen(hash);
            //print_on_screen(file_name);
            vector<string> list=split_string(line);
            for(int i=0;i<list.size();i++){
                access_seeders("share",file_name,file_hash,list[i],-1); // send dummy socket id
            }

        }
    in.close();
    }


    //cout<<response<<"\n"; // ok message
}


int main(int argc,const char *argv[]){

    my_address=argv[1];
    other_tracker_address=argv[2];

    seeder_list_file_path=argv[3];
    log_file_path=argv[4];

    string my_ip=get_ip_address(my_address);
    int my_port=stoi(get_port_address(my_address));

    //cout<<my_address<<"\n";
    //cout<<other_tracker_address<<"\n";
    //cout<<seeder_list_file_path<<"\n";
    //cout<<log_file_path<<"\n";

    synchronize_with_other_tracker();

    int server_socket_id, client_socket_id;
    unsigned int server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;

    //unlink("server_socket");
    server_socket_id = socket(AF_INET, SOCK_STREAM, 0);

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(my_ip.c_str());  // getting ip and port from user
    server_address.sin_port = my_port;
    server_len = sizeof(server_address);
    bind(server_socket_id, (struct sockaddr *)&server_address, server_len);

    listen(server_socket_id, 5);
    signal(SIGCHLD, SIG_IGN);
    
    while(1) {
        char ch;
        printf("server waiting\n");

        client_len = sizeof(client_address);
        client_socket_id = accept(server_socket_id,(struct sockaddr *)&client_address, &client_len);
        thread th(serve, client_socket_id);
        th.detach();
    }
    
    
}