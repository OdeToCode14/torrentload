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

using namespace std;


void serve(int client_sockfd){
    //cout<<"here\n";
    //cout<<client_sockfd<<"\n";
    char buffer[256];
    int BUFFER_SIZE=256;
    int len = recv(client_sockfd, buffer, BUFFER_SIZE, 0);
    buffer[len]='\0';
    string message=(string) buffer;
    cout<<buffer<<"\n";
    for(int i=0;i<999999999;i++){
        //cout<<"this is "<<client_sockfd<<"\n";
    }
    cout<<"processed\n";
    close(client_sockfd);
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

    int server_sockfd, client_sockfd;
    unsigned int server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;

    unlink("server_socket");
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address.sin_port = 9735;
    server_len = sizeof(server_address);
    bind(server_sockfd, (struct sockaddr *)&server_address, server_len);

    listen(server_sockfd, 5);
    signal(SIGCHLD, SIG_IGN);
    
    while(1) {
        char ch;
        printf("server waiting\n");

        client_len = sizeof(client_address);
        client_sockfd = accept(server_sockfd,(struct sockaddr *)&client_address, &client_len);
        thread th(serve, client_sockfd);
        th.detach();
    }
    
    /*
    while(1){
        char ch;
        printf("server waiting\n");

        client_len = sizeof(client_address);
        client_sockfd = accept(server_sockfd,(struct sockaddr *)&client_address, &client_len);

        if(fork() == 0){
            serve(client_sockfd);
            close(client_sockfd);
            exit(0);
        }
        else {
            close(client_sockfd);
        }
    }
    */
}