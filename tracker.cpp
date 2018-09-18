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
#define pb push_back
#define mk make_pair
using namespace std;

/*
void serve(int client_socket_id){
    //cout<<"here\n";
    //cout<<client_socket_id<<"\n";
    char buffer[256];
    int BUFFER_SIZE=256;
    int len = recv(client_socket_id, buffer, BUFFER_SIZE, 0);
    buffer[len]='\0';
    string message=(string) buffer;
    cout<<buffer<<"\n";
    for(int i=0;i<999999999;i++){
        //cout<<"this is "<<client_socket_id<<"\n";
    }
    cout<<"processed\n";
    close(client_socket_id);
}
*/

//map<string,vector<pair<string,string>>> seeders;
map<string,map<string,string>> seeders;
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
    cout<<"received "<<length<<" "<<message<<"\n";
    return message;
}

void serve(int client_socket_id){
    //cout<<"here\n";
    //cout<<client_socket_id<<"\n";
    
    string message=receive_message(client_socket_id);
    if(message == "share"){
        string response="ok";
        send_message(client_socket_id,response);
        string file_name=receive_message(client_socket_id);
        send_message(client_socket_id,response);
        string file_hash=receive_message(client_socket_id); // this is hash of hash
        send_message(client_socket_id,response);
        string address=receive_message(client_socket_id);
        send_message(client_socket_id,response);
        
        //cout<<file_name <<" "<< file_hash << " "<<address<<"\n";
        /*
        if(seeders.find(file_hash) != seeders.end()){
            seeders[file_hash].pb(mk(file_name,address));
        }
        else{
            seeders[file_hash]=vector<pair<string,string>>();
            seeders[file_hash].pb(mk(file_name,address));
        }
        */
        if(seeders.find(file_hash) != seeders.end()){
            seeders[file_hash][address]=file_name;
        }
        else{
        /*
            seeders[file_hash]=vector<pair<string,string>>();
            seeders[file_hash].pb(mk(file_name,address));
        */
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
        /*
        if(seeders.find(file_hash) != seeders.end()){
            string seeder_list="";
            for(int i=0;i<seeders[file_hash].size();i++){
                seeder_list = seeder_list + seeders[file_hash][i].second+" ";
            }
            send_message(client_socket_id,seeder_list);
        }
        else{
            send_message(client_socket_id,"not found");
        }
        */
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
            /*
            for(int i=0;i<seeders[file_hash].size();i++){
                if(seeders[file_hash][i].second == address){
                    seeders[file_hash].erase(seeders[file_hash].begin()); //remove all entries
                }
            }
            if(seeders.find(file_hash) == seeders.end()){
                cout<<"done\n";
            }
            else{
                cout<<"this "<<seeders[file_hash][0].first<<"\n";
            }*/
        }
        else{
            send_message(client_socket_id,"file not found");
        }   
    }
    cout<<"processed\n";
    close(client_socket_id);
}

int main(int argc,const char *argv[]){

    string my_address=argv[1];
    string other_tracker_address=argv[2];

    string seeder_list_file_path=argv[3];
    string log_file_path=argv[4];
    cout<<my_address<<"\n";
    cout<<other_tracker_address<<"\n";
    cout<<seeder_list_file_path<<"\n";
    cout<<log_file_path<<"\n";

    int server_socket_id, client_socket_id;
    unsigned int server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;

    unlink("server_socket");
    server_socket_id = socket(AF_INET, SOCK_STREAM, 0);

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address.sin_port = 9735;
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
    
    /*
    while(1){
        char ch;
        printf("server waiting\n");

        client_len = sizeof(client_address);
        client_socket_id = accept(server_socket_id,(struct sockaddr *)&client_address, &client_len);

        if(fork() == 0){
            serve(client_socket_id);
            close(client_socket_id);
            exit(0);
        }
        else {
            close(client_socket_id);
        }
    }
    */
}