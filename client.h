#ifndef CLIENT_H
#define CLIENT_H

#include "create_sha.h"
#include "client.h"

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

using namespace std;

//of uploader

extern map<string,pair<string,string>> seeded_list;

void serve(int client_socket_id);
void listen_download_requests(string my_address, string tracker_one_address,string tracker_two_address,string log_file_path);

//of client
void send_message(int socket_id,string message);
string receive_message(int socket_id);
string get_ip_address(string address);
string get_port_address(string address);
vector<string> split_string(string command);
void print_on_screen(string str);
string access_seeded_list(string operation,string hash_of_hash,string file_path,string bit_map);
#endif