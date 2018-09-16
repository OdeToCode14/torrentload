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
    cout<<"here";
    char buffer[256];
    int BUFFER_SIZE=256;
    FILE *fd = fopen("abc.txt", "rb");
    size_t rret, wret;
    int bytes_read;
    while (!feof(fd)) {
        if ((bytes_read = fread(&buffer, 1, BUFFER_SIZE, fd)) > 0)
            send(client_sockfd, buffer, bytes_read, 0);
        else
            break;
    }
    fclose(fd);
            /*
            read(client_sockfd, &ch, 1);
            sleep(5);
            ch++;
            write(client_sockfd, &ch, 1);
            */
    close(client_sockfd);
}

int main()
{
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
        th.join();
    }
}