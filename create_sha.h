#ifndef CREATE_SHA_H
#define CREATE_SHA_H

#include "create_sha.h"

#include <bits/stdc++.h>
#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>

#include <iostream>
#include <fstream>
#define chunk_size (1<<19)
//#define chunk_size (3)
using namespace std;

string get_complete_sha(string word);
string get_hash(string path);
string get_file_name_from_path(string current);

#endif