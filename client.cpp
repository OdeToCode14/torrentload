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

using namespace std;
string my_address;

string tracker_one_address;
string tracker_two_address;
string log_file_path;

int tracker_socket_id;

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
    int tracker_socket_id;
    int len;
    struct sockaddr_in address;
    int result;
    tracker_socket_id = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1"); //add tracker address here
    address.sin_port = 9735;
    len = sizeof(address);

    result = connect(tracker_socket_id, (struct sockaddr *)&address, len);
    if(result == -1) {
        perror("oops: client1");
        exit(1);
    }
    return tracker_socket_id;
}

void command_mode(){
    while(1){
        string command;
        cin>>command;

        int tracker_socket_id=get_tracker_connection();

        if(command == "share"){
            send_message(tracker_socket_id,command);
            string response=receive_message(tracker_socket_id);
            cout<<response<<"\n";
            send_message(tracker_socket_id,"ahg.txt");
            response=receive_message(tracker_socket_id);
            cout<<response<<"\n";
            send_message(tracker_socket_id,"0xfh");
            response=receive_message(tracker_socket_id);
            cout<<response<<"\n";
            send_message(tracker_socket_id,"125:45:");
            response=receive_message(tracker_socket_id);
            cout<<response<<"\n";
        }
        else if(command == "get"){
            send_message(tracker_socket_id,command);
            string response=receive_message(tracker_socket_id);
            cout<<response<<"\n";
            send_message(tracker_socket_id,"0xfh");
            response=receive_message(tracker_socket_id);
            cout<<response<<"\n";
        }

        else if(command == "remove"){
            send_message(tracker_socket_id,command);
            string response=receive_message(tracker_socket_id);
            cout<<response<<"\n";
            send_message(tracker_socket_id,"0xfh");
            response=receive_message(tracker_socket_id);
            cout<<response<<"\n";
            send_message(tracker_socket_id,"125:45:");
            response=receive_message(tracker_socket_id);
            cout<<response<<"\n";
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

    /*
    write(sockfd, &ch, 1);
    read(sockfd, &ch, 1);
    printf("char from server = %c\n", ch);
    */
    /*
    size_t datasize;
    FILE* fd = fopen("file", "wb");
    char buffer[256];
    int BUFFER_SIZE=256;
    while (datasize)
    {
        datasize = recv(sockfd, buffer, BUFFER_SIZE, 0);
        fwrite(&buffer, 1, datasize, fd);
    }
    fclose(fd);
    */
    command_mode();

    close(tracker_socket_id);
}