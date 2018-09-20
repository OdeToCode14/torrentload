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

mutex mtx;

string my_address;
string other_tracker_address;

//map<string,vector<pair<string,string>>> seeders;
map<string,map<string,string>> seeders;

void print_on_screen(string str){
    mtx.lock();
        cout<<str<<"\n";
    mtx.unlock();
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

void serve(int client_socket_id){
    //cout<<"here\n";
    //cout<<client_socket_id<<"\n";
    
    string message=receive_message(client_socket_id);
    if(message == "synchronize"){
        //string response="ok";
        //send_message(client_socket_id,response);
        //cout<<seeders.size()<<"\n";
        string response="";
        for(auto mp : seeders){
            string hash=mp.first;
            string list=hash+"-";
            for(auto it : mp.second){
                list=list + "(" + it.first + "," +it.second + ")";
            }
            send_message(client_socket_id,list);
            //cout<<"sending "<<list<<"\n";
            print_on_screen("sending "+list);

            receive_message(client_socket_id);
        }
        send_message(client_socket_id,"done");
        
        //cout<<"synchronized\n";
        return;
    }
    else if(message == "share"){
        string response="ok";
        send_message(client_socket_id,response);
        string file_name=receive_message(client_socket_id);
        send_message(client_socket_id,response);
        string file_hash=receive_message(client_socket_id); // this is hash of hash
        send_message(client_socket_id,response);
        string address=receive_message(client_socket_id);
        send_message(client_socket_id,response);
        
        //cout<<file_name <<" "<< file_hash << " "<<address<<"\n";
        
        if(seeders.find(file_hash) != seeders.end()){
            seeders[file_hash][address]=file_name;
        }
        else{
            seeders[file_hash]=map<string,string>();
            seeders[file_hash][address]=file_name;
        }
        //cout<<"this one "<<seeder[file_hash][address]<<"\n";

    }
    else if(message == "get"){
        string response="ok";
        send_message(client_socket_id,response);
        string file_hash=receive_message(client_socket_id); // this is hash of hash
        //send_message(client_socket_id,response);
        
        //cout<<file_name <<" "<< file_hash << " "<<address<<"\n";
        

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
    else if(message == "remove"){
        string response="ok";
        send_message(client_socket_id,response);
        string file_hash=receive_message(client_socket_id); // this is hash of hash
        send_message(client_socket_id,response);
        string address=receive_message(client_socket_id);
        //send_message(client_socket_id,response);
        
        //cout<<file_name <<" "<< file_hash << " "<<address<<"\n";
        
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
    //cout<<"processed\n";
    print_on_screen("processed");
    close(client_socket_id);
}
/*
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

void synchronize_with_other_tracker(){
    int other_tracker_socket_id=get_other_tracker_connection();
    if(other_tracker_socket_id == -1){
        cout<<"Other tracker is down\n";
        return;
    }
    send_message(other_tracker_socket_id,"synchronize");
    string response=receive_message(other_tracker_socket_id);
    cout<<"receiving " <<response<<"\n";
    while(response != "done"){
        //cout<<"received " <<response<<"\n";
        send_message(other_tracker_socket_id,"continue");
        response=receive_message(other_tracker_socket_id);
        if(response != "done"){
            int ind=response.find("-");
            string hash=response.substr(0,ind);
            seeders[hash]=map<string,string>();
        }
        cout<<"receiving " <<response<<"\n";
    }
    //cout<<response<<"\n"; // ok message
}
*/

int main(int argc,const char *argv[]){

    my_address=argv[1];
    other_tracker_address=argv[2];

    string seeder_list_file_path=argv[3];
    string log_file_path=argv[4];

    string my_ip=get_ip_address(my_address);
    int my_port=stoi(get_port_address(my_address));

    //cout<<my_address<<"\n";
    //cout<<other_tracker_address<<"\n";
    //cout<<seeder_list_file_path<<"\n";
    //cout<<log_file_path<<"\n";

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

    //synchronize_with_other_tracker();
    
    while(1) {
        char ch;
        printf("server waiting\n");

        client_len = sizeof(client_address);
        client_socket_id = accept(server_socket_id,(struct sockaddr *)&client_address, &client_len);
        thread th(serve, client_socket_id);
        th.detach();
    }
    
    
}