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

#include "create_sha.h"
#include "client.h"

using namespace std;
string my_address;

string tracker_one_address;
string tracker_two_address;
string log_file_path;

int tracker_socket_id;


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
        perror("oops: client1");
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
        cout<<"file does not exist\n";
        return;
    }

    string hash=get_hash(file_path);
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
    cout<<response<<"\n";

    send_message(tracker_socket_id,file_name);
    response=receive_message(tracker_socket_id);
    cout<<response<<"\n";
    send_message(tracker_socket_id,hash_of_hash);
    response=receive_message(tracker_socket_id);
    cout<<response<<"\n";
    send_message(tracker_socket_id,my_address);
    response=receive_message(tracker_socket_id);
    cout<<response<<"\n";

    //start seeding 
}

string get_last_line(ifstream& in){
    string line;
    while (in >> ws && getline(in, line)); // skip empty lines

    return line;
}

void request_download(string response){
    vector<string> processed_response=split_string(response);

    string other_client_ip=get_ip_address(processed_response[0]);
    int other_client_port=stoi(get_port_address(processed_response[0]));

    cout<<"connecting to "<<other_client_ip <<":"<<other_client_port<<"\n";
    
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
        perror("oops: client1");
        exit(1);
    }
    send_message(other_client_socket_id,"hi there dude");
    size_t datasize;
    FILE* fd = fopen("down_text", "wb");
    char buffer[256];
    int BUFFER_SIZE=256;
    while (datasize)
    {
        datasize = recv(other_client_socket_id, buffer, BUFFER_SIZE, 0);
        fwrite(&buffer, 1, datasize, fd);
    }
    fclose(fd);

    close(other_client_socket_id);
}

void get_file(int tracker_socket_id, vector<string> processed_command){  // incomplete code
    string torrent_file_path=processed_command[1];
    string destination_path=processed_command[2];

    string hash;
    ifstream in(torrent_file_path);
    if (in){
        hash = get_last_line(in);
        cout <<"here "<< hash << '\n';
    }
    else{
        cout << "Unable to open torrent file.\n";
        return;
    }

    string hash_of_hash=get_complete_sha(hash);

    send_message(tracker_socket_id,processed_command[0]);
    string response=receive_message(tracker_socket_id);
    cout<<response<<"\n";
    send_message(tracker_socket_id,hash_of_hash);
    response=receive_message(tracker_socket_id);
    cout<<response<<"\n";
    // check for error message before contacting
    if(response == "not found"){
        cout<<"No one in the network has this file right now. try after sometime\n";
    }
    else{
        request_download(response);
    }

    // contact response list
}

void remove_file(int tracker_socket_id, vector<string> processed_command){
    string torrent_file_path=processed_command[1];
    //cout<<"removing "<<torrent_file_path<<"\n";
    string hash;
    ifstream in(torrent_file_path);
    if (in){
        hash = get_last_line(in);
        cout <<"here "<< hash << '\n';
    }
    else{
        cout << "Unable to open torrent file.\n";
        return;
    }
    string hash_of_hash=get_complete_sha(hash);
    send_message(tracker_socket_id,processed_command[0]);
    string response=receive_message(tracker_socket_id);
    cout<<response<<"\n";
    send_message(tracker_socket_id,hash_of_hash);
    response=receive_message(tracker_socket_id);
    cout<<response<<"\n";
    send_message(tracker_socket_id,my_address);
    response=receive_message(tracker_socket_id);
    cout<<response<<"\n";
    if (remove(torrent_file_path.c_str()) == 0)
        cout<<"removed sucessfully\n";
    else
        cout<<"cannot remove\n";
}

void command_mode(){
    while(1){
        string command;
        getline(cin,command);

        int tracker_socket_id=get_tracker_connection();

        vector<string> processed_command=split_string(command);
        cout<<processed_command.size();
        
        if(processed_command[0] == "share"){
            share_file(tracker_socket_id,processed_command);
        }
        else if(processed_command[0] == "get"){
            get_file(tracker_socket_id,processed_command);
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

    close(tracker_socket_id);
}