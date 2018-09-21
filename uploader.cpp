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

void serve(int client_socket_id){
	string hash_of_hash=receive_message(client_socket_id); // hash_of_hash of file requested
	//cout<<message<<"\n";
	//cout<<"in service\n";
	string file_path=access_seeded_list("get file path",hash_of_hash,"");
	char buffer[256];
	int BUFFER_SIZE=256;
	FILE *fd = fopen(file_path.c_str(), "rb");
	size_t rret, wret;
	int bytes_read;
	while (!feof(fd)) {
	    if ((bytes_read = fread(&buffer, 1, BUFFER_SIZE, fd)) > 0)
	        send(client_socket_id, buffer, bytes_read, 0);
	    else
	        break;
	}
    fclose(fd);

	close(client_socket_id);
}

void listen_download_requests(string my_address, string tracker_one_address,string tracker_two_address,string log_file_path){
	//cout<<"started\n";

	string my_ip=get_ip_address(my_address);
    int my_port=stoi(get_port_address(my_address));

	int server_socket_id, client_socket_id;
    unsigned int server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;

    server_socket_id = socket(AF_INET, SOCK_STREAM, 0);

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(my_ip.c_str());
    server_address.sin_port = my_port;
    server_len = sizeof(server_address);
    bind(server_socket_id, (struct sockaddr *)&server_address, server_len);

    listen(server_socket_id, 5);
    signal(SIGCHLD, SIG_IGN);
    
    while(1) {
        char ch;
        //printf("server waiting\n");
        print_on_screen("server waiting");
        client_len = sizeof(client_address);
        client_socket_id = accept(server_socket_id,(struct sockaddr *)&client_address, &client_len);
        thread th(serve, client_socket_id);
        th.detach();
    }
}