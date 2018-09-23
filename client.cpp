#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <thread>
#include <string>
#include <bits/stdc++.h>
#include <mutex> 

#include "create_sha.h"
#include "client.h"

using namespace std;

mutex output_device_mtx;
mutex seeded_list_mtx;
mutex download_list_mtx;

map<string,pair<string,string>> seeded_list; // key is file hash . first part of pair is file path second bit map
map<string,string> download_list; // key is file path. second part download status . key is filepath and not file hash as multiple files can be downloaded at different paths
string my_address;

string tracker_one_address;
string tracker_two_address;
string log_file_path;

//int tracker_socket_id;


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
    //cout<<"sending "<< message<<"\n";
    send(socket_id, message.c_str(), message.length()+1, 0);
}
string receive_message(int socket_id){
    char buffer[256];
    int BUFFER_SIZE=256;
    int length = recv(socket_id, buffer, BUFFER_SIZE, 0);
    buffer[length]='\0';
    string message=(string) buffer;
    return message;
}

int get_tracker_connection(){
    string tracker_one_ip=get_ip_address(tracker_one_address);
    int tracker_one_port=stoi(get_port_address(tracker_one_address));

    int tracker_socket_id;
    int len;
    struct sockaddr_in address;
    int result;
    tracker_socket_id = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(tracker_one_ip.c_str()); //add tracker address here
    address.sin_port = tracker_one_port;
    len = sizeof(address);

    result = connect(tracker_socket_id, (struct sockaddr *)&address, len);
    if(result == -1) {
        //perror("oops: client1");
        print_on_screen("cannot connect");
        exit(1);
    }
    return tracker_socket_id;
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

string access_download_list(string operation,string file_path){
    lock_guard<mutex> guard(download_list_mtx);
        if(operation == "add"){
            download_list[file_path]="[D]";
        }
        else if(operation=="downloaded"){
            download_list[file_path]="[S]";
        }
        else if(operation == "show downloads"){
            print_on_screen("DOWNLOAD LIST");
            for(auto it : download_list){
                print_on_screen(it.second + " " + it.first);
                print_on_screen("");
            }
        }
}

 string access_seeded_list(string operation,string hash_of_hash,string file_path,string bit_map){
    lock_guard<mutex> guard(seeded_list_mtx);
        if(operation == "add"){
            //print_on_screen(hash_of_hash+" "+file_path+" "+bit_map);
            seeded_list[hash_of_hash]=make_pair(file_path,bit_map);
            //print_on_screen("done sucessfully");
        }
        else if(operation == "remove"){
            if(seeded_list.find(hash_of_hash) != seeded_list.end()){
                seeded_list.erase(hash_of_hash);
            }
        }
        else if(operation == "get file path"){
            if(seeded_list.find(hash_of_hash) != seeded_list.end()){
                return seeded_list[hash_of_hash].first;
            }
        }
        else if(operation == "get bit map"){
            if(seeded_list.find(hash_of_hash) != seeded_list.end()){
                return seeded_list[hash_of_hash].second;
            }
        }
    //mtx.unlock();
 }

void share_file(int tracker_socket_id , vector<string> processed_command){ // incomplete

    string file_path=processed_command[1];
    string torrent_file_name=processed_command[2];

    string file_name=get_file_name_from_path(file_path);
    //cout<<name<<"\n";
    streampos file_size;

    ifstream file (file_path, ios::in|ios::binary|ios::ate);
    if (file.is_open()){
        file_size = file.tellg();   
        file.seekg (0, ios::beg);
        file.close();
    }
    else{
        print_on_screen("file does not exist");
        close(tracker_socket_id);
        return;
    }

    string hash=get_hash(file_path);
    int number_of_chunks=hash.length()/20;
    string bit_map="";                     // all 1s since it has complete file
    for(int i=0;i<number_of_chunks;i++){
        bit_map=bit_map+"1";
    }
    //string hash_of_hash=();

    ofstream torrent_file;
    torrent_file.open (torrent_file_name.c_str());

    torrent_file << tracker_one_address <<"\n";
    torrent_file << tracker_two_address <<"\n";
    torrent_file << file_name <<"\n";
    torrent_file << file_size <<"\n";
    torrent_file << hash <<"\n";
    torrent_file.close();
    //cout<<hash<<"\n";
    string hash_of_hash=get_complete_sha(hash);


            

    send_message(tracker_socket_id,processed_command[0]);
    string response=receive_message(tracker_socket_id);
    print_on_screen(response);

    send_message(tracker_socket_id,file_name);
    response=receive_message(tracker_socket_id);
    print_on_screen(response);
    send_message(tracker_socket_id,hash_of_hash);
    response=receive_message(tracker_socket_id);
    print_on_screen(response);
    send_message(tracker_socket_id,my_address);
    response=receive_message(tracker_socket_id);
    print_on_screen(response);

    access_seeded_list("add",hash_of_hash,file_path,bit_map);
    //start seeding 
    close(tracker_socket_id);
}

string get_last_line(ifstream& in){
    string line;
    while (in >> ws && getline(in, line)); // skip empty lines

    return line;
}

/*void request_download(string response,string hash_of_hash,string destination_path){
    vector<string> processed_response=split_string(response);

    string other_client_ip=get_ip_address(processed_response[0]);
    int other_client_port=stoi(get_port_address(processed_response[0]));

    //cout<<"connecting to "<<other_client_ip <<":"<<other_client_port<<"\n";
    //print_on_screen("connecting to " + other_client_ip + ":" + other_client_port);
    int other_client_socket_id;
    int len;
    struct sockaddr_in address;
    int result;
    other_client_socket_id = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(other_client_ip.c_str()); //add tracker address here
    address.sin_port = other_client_port;
    len = sizeof(address);

    result = connect(other_client_socket_id, (struct sockaddr *)&address, len);
    if(result == -1) {
        //perror("oops: client1");
        print_on_screen("cannot connect");
        exit(1);
    }
    send_message(other_client_socket_id,hash_of_hash);
    size_t datasize;
    FILE* fd = fopen(destination_path.c_str(), "wb");
    char buffer[256];
    int BUFFER_SIZE=256;
    while (datasize)
    {
        datasize = recv(other_client_socket_id, buffer, BUFFER_SIZE, 0);
        fwrite(&buffer, 1, datasize, fd);
    }
    fclose(fd);

    close(other_client_socket_id);
    print_on_screen("Downloaded sucessfully");
}*/

void request_download(string client_address,string hash_of_hash,string destination_path,vector<int> chunk_numbers){
    //vector<string> processed_response=split_string(response);
    string chunk_numbers_str="";
    for(int i=0;i<chunk_numbers.size();i++){
        chunk_numbers_str=chunk_numbers_str+to_string(chunk_numbers[i])+" ";
    }
    string other_client_ip=get_ip_address(client_address);
    int other_client_port=stoi(get_port_address(client_address));

    //cout<<"connecting to "<<other_client_ip <<":"<<other_client_port<<"\n";
    //print_on_screen("connecting to " + other_client_ip + ":" + other_client_port);
    int other_client_socket_id;
    int len;
    struct sockaddr_in address;
    int result;
    other_client_socket_id = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(other_client_ip.c_str()); //add tracker address here
    address.sin_port = other_client_port;
    len = sizeof(address);

    result = connect(other_client_socket_id, (struct sockaddr *)&address, len);
    if(result == -1) {
        //perror("oops: client1");
        print_on_screen("cannot connect");
        exit(1);
    }
    send_message(other_client_socket_id,"download");
    string reply=receive_message(other_client_socket_id);
    send_message(other_client_socket_id,hash_of_hash);
    string response=receive_message(other_client_socket_id);
    print_on_screen(response);
    send_message(other_client_socket_id,chunk_numbers_str);
    size_t datasize;
    FILE* fd = fopen(destination_path.c_str(), "r+b");
    for(int i=0;i<chunk_numbers.size();i++){
        char buffer[chunk_size + 1];
        int BUFFER_SIZE=chunk_size;
        
            int skip=chunk_numbers[i]*chunk_size;
            //print_on_screen("skipping "+to_string(skip));
            datasize = recv(other_client_socket_id, buffer, BUFFER_SIZE, 0);
            //print_on_screen("*****");
            fseek ( fd , skip , SEEK_SET );
            //print_on_screen("*****");
            fwrite(&buffer, 1, datasize, fd);
            //print_on_screen("*****");
            //print_on_screen("written this much "+to_string(datasize));
            //print_on_screen(buffer);
        
    }
    fclose(fd);

    close(other_client_socket_id);
    //print_on_screen("Downloaded sucessfully");
}

void start_seeding(string hash_of_hash, string file_path,int number_of_chunks){
    string bit_map="";
    for(int i=0;i<number_of_chunks;i++){
        bit_map=bit_map+"0";
    }
    access_seeded_list("add",hash_of_hash,file_path,bit_map);
    //print_on_screen("done sucessfully");
}
string get_bit_map_from_client(string client_address,string hash_of_hash){
    string other_client_ip=get_ip_address(client_address);
    int other_client_port=stoi(get_port_address(client_address));
    int other_client_socket_id;
    int len;
    struct sockaddr_in address;
    int result;
    other_client_socket_id = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(other_client_ip.c_str()); //add tracker address here
    address.sin_port = other_client_port;
    len = sizeof(address);

    result = connect(other_client_socket_id, (struct sockaddr *)&address, len);
    if(result == -1) {
        //perror("oops: client1");
        print_on_screen("cannot connect");
        exit(1);
    }
    send_message(other_client_socket_id,"get bit map");
    receive_message(other_client_socket_id);
    send_message(other_client_socket_id,hash_of_hash);
    string response=receive_message(other_client_socket_id);
    print_on_screen("got this bit_map "+response);
    return response;
}

void get_file(int tracker_socket_id, vector<string> processed_command){  // incomplete code
    print_on_screen("working here");
    string torrent_file_path=processed_command[1];
    string destination_path=processed_command[2];
    
    string hash;
    ifstream in(torrent_file_path);
    if (in){
        hash = get_last_line(in);
        //cout <<"here "<< hash << '\n';
    }
    else{
        print_on_screen("Unable to open torrent file.");
        close(tracker_socket_id);
        return;
    }
    int number_of_chunks=hash.length()/20;
    string hash_of_hash=get_complete_sha(hash);
    print_on_screen(processed_command[0]);
    send_message(tracker_socket_id,processed_command[0]);
    string response=receive_message(tracker_socket_id);
    print_on_screen("resp is "+response);
    //print_on_screen("here");
    send_message(tracker_socket_id,hash_of_hash);
    response=receive_message(tracker_socket_id);
    print_on_screen(response);
    // check for error message before contacting
    if(response == "not found"){
        print_on_screen("No one in the network has this file right now. try after sometime");
    }
    else{ //code for deciding chunk distribution

        FILE* fd = fopen(destination_path.c_str(), "w");  //create the file first
        fclose(fd);
        //start_seeding(hash_of_hash,destination_path,number_of_chunks); // start seeding this file
        vector<string> seeder_list=split_string(response);
        vector<string> bit_map_list=split_string(response);
        int number_of_seeders=seeder_list.size();
        vector<string> bit_maps_of_seeders(number_of_seeders);
        for(int i=0;i<number_of_seeders;i++){
            string bit_map=get_bit_map_from_client(seeder_list[i],hash_of_hash);
            bit_map_list[i]=bit_map;
            //print_on_screen("seeders are "+seeder_list[i]);
            //print_on_screen("bit_map "+bit_map);
        }

        vector<thread> thrs;

        vector<vector<int>> chunks_from_seeder(number_of_seeders);
        for(int i=0;i<number_of_chunks;i++){
            int start=rand()%number_of_seeders ;
            print_on_screen("start is "+to_string(start));
            int use=start;
            while(true){
                if(bit_map_list[use][i] == '1'){
                    chunks_from_seeder[use].push_back(i);
                    break;
                }
                else{
                    use=(use+1)%number_of_seeders;
                    if(use == start){
                        print_on_screen("chunk missing");
                        break;
                    }
                }
            }
        }
        access_download_list("add",destination_path);
        //access_download_list("show downloads",destination_path);
        for(int i=0;i<number_of_seeders;i++){
            if(chunks_from_seeder[i].size() > 0){
                thrs.push_back(thread(request_download,seeder_list[i],hash_of_hash,destination_path,chunks_from_seeder[i]));
            }
        }
        

        //thread th(wait_for_download,ref(thrs));
        //th.detach();
        for(int i=0;i<thrs.size();i++){
            thrs[i].join();
        }
        access_download_list("downloaded",destination_path);

       /* vector<int> chunk_numbers_one;
        //print_on_screen("chunk_size " + to_string(chunk_size) + "lenght of hash " + to_string(hash.length()));
        //print_on_screen("number of chunks "+to_string(number_of_chunks));
        for(int i=0;i<number_of_chunks/2;i++){
            chunk_numbers_one.push_back(i);
            //print_on_screen("chunk_numbers " + to_string(chunk_numbers_one[i]));
        }
        vector<int> chunk_numbers_two;
        for(int i=number_of_chunks/2;i<number_of_chunks;i++){
            chunk_numbers_two.push_back(i);
        }
        //request_download(seeder_list[0],hash_of_hash,destination_path,chunk_numbers_one);
        //request_download(seeder_list[1],hash_of_hash,destination_path,chunk_numbers_two);
        thread th1(request_download,seeder_list[0],hash_of_hash,destination_path,chunk_numbers_one);
        thread th2(request_download,seeder_list[1],hash_of_hash,destination_path,chunk_numbers_two);
        th1.join();
        th2.join();
        */
    }
    close(tracker_socket_id);

    // contact response list
}

void remove_file(int tracker_socket_id, vector<string> processed_command){
    string torrent_file_path=processed_command[1];
    //cout<<"removing "<<torrent_file_path<<"\n";
    string hash;
    ifstream in(torrent_file_path);
    if (in){
        hash = get_last_line(in);
        //print_on_screen("here " + hash );
    }
    else{
        print_on_screen("Unable to open torrent file.");
        close(tracker_socket_id);
        return;
    }
    string hash_of_hash=get_complete_sha(hash);
    send_message(tracker_socket_id,processed_command[0]);
    string response=receive_message(tracker_socket_id);
    print_on_screen(response);
    send_message(tracker_socket_id,hash_of_hash);
    response=receive_message(tracker_socket_id);
    print_on_screen(response);
    send_message(tracker_socket_id,my_address);
    response=receive_message(tracker_socket_id);
    print_on_screen(response);
    if (remove(torrent_file_path.c_str()) == 0){
        print_on_screen("removed sucessfully");
        access_seeded_list("remove",hash_of_hash,torrent_file_path,"");
    }
    else{
        print_on_screen("cannot remove");
    }
    close(tracker_socket_id);
}

void command_mode(){ // give the responsibilty of closing socket to function called
    while(1){
        string command;
        getline(cin,command);

        int tracker_socket_id=get_tracker_connection();
        print_on_screen("connected");

        vector<string> processed_command=split_string(command);
        //cout<<processed_command.size();
        //print_on_screen(processed_command[0]);
        if(processed_command[0] == "share"){
            share_file(tracker_socket_id,processed_command);
        }
        else if(processed_command[0] == "get"){
            //get_file(tracker_socket_id,processed_command);
            thread th(get_file,tracker_socket_id,processed_command);
            th.detach();          // seperate thread for each download
        }

        else if(processed_command[0] == "remove"){
            remove_file(tracker_socket_id,processed_command);
/*
            send_message(tracker_socket_id,command);
            string response=receive_message(tracker_socket_id);
            cout<<response<<"\n";
            send_message(tracker_socket_id,"0xfh");
            response=receive_message(tracker_socket_id);
            cout<<response<<"\n";
            send_message(tracker_socket_id,"125:45:");
            response=receive_message(tracker_socket_id);
            cout<<response<<"\n";*/
        }
        else if(processed_command[0] == "show" && processed_command[1]=="downloads"){
            //print_on_screen("here dude "+processed_command[0]);
            access_download_list("show downloads","");
            close(tracker_socket_id);
        }
        //close(tracker_socket_id);  // give the responsibilty of closing socket to function called
        //print_on_screen("closed");
    }
}

int main(int argc,const char *argv[]){
    my_address=argv[1];

    tracker_one_address=argv[2];
    tracker_two_address=argv[3];
    log_file_path=argv[4];

    //create a log file

    //code for sharing currently available mtorrents

    //start thread for acting as server

    
    // specify my address to uploader
    thread th(listen_download_requests, my_address, tracker_one_address, tracker_two_address, log_file_path);
    th.detach();
    command_mode();

    //close(tracker_socket_id);
}