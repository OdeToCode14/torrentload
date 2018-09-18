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

void serve(int client_socket_id){
	cout<<"in service\n";
}

void listen_download_requests(string my_address, string tracker_one_address,string tracker_two_address,string log_file_path){
	cout<<"started\n";
	int server_socket_id, client_socket_id;
    unsigned int server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;

    server_socket_id = socket(AF_INET, SOCK_STREAM, 0);

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address.sin_port = 9737;
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