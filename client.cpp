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

int main()
{
int server_socket_id;
int len;
struct sockaddr_in address;
int result;
char ch = 'A';

server_socket_id = socket(AF_INET, SOCK_STREAM, 0);

address.sin_family = AF_INET;
address.sin_addr.s_addr = inet_addr("127.0.0.1");
address.sin_port = 9735;
len = sizeof(address);

result = connect(server_socket_id, (struct sockaddr *)&address, len);
if(result == -1) {
perror("oops: client1");
exit(1);
}
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
string command;
cin>>command;
if(command == "share"){
    send_message(server_socket_id,command);
    string response=receive_message(server_socket_id);
    cout<<response<<"\n";
    send_message(server_socket_id,"ahg.txt");
    response=receive_message(server_socket_id);
    cout<<response<<"\n";
    send_message(server_socket_id,"0xfh");
    response=receive_message(server_socket_id);
    cout<<response<<"\n";
    send_message(server_socket_id,"125:45:");
    response=receive_message(server_socket_id);
    cout<<response<<"\n";
}
else if(command == "get"){
    send_message(server_socket_id,command);
    string response=receive_message(server_socket_id);
    cout<<response<<"\n";
    send_message(server_socket_id,"0xfh");
    response=receive_message(server_socket_id);
    cout<<response<<"\n";
}

else if(command == "remove"){
    send_message(server_socket_id,command);
    string response=receive_message(server_socket_id);
    cout<<response<<"\n";
    send_message(server_socket_id,"0xfh");
    response=receive_message(server_socket_id);
    cout<<response<<"\n";
    send_message(server_socket_id,"125:45:");
    response=receive_message(server_socket_id);
    cout<<response<<"\n";
}

close(server_socket_id);
}