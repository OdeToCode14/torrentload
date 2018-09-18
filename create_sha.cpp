#include <bits/stdc++.h>
#include <stdio.h>
#include <string>
#include <openssl/sha.h>

#include <iostream>
#include <fstream>

#define chunk_size (1<<19)

using namespace std;


string get_complete_sha(string word){
	string sha1_hash="";
	const char * arr=word.c_str();
	unsigned char out[20];

	SHA1((unsigned char*)arr, strlen(arr),out);
	stringstream s;
	for (int i = 0; i < 20; ++i) 
		s << hex << setfill('0') << setw(2) << (unsigned short) out[i];
		        //cout << s.str() << "\n";
	string hash=s.str();
	return hash;
}

string get_hash(string path){
	string sha1_hash="";
	streampos size;
	streampos last_chunk_size;
	  ifstream file (path, ios::in|ios::binary|ios::ate);
	  if (file.is_open()){
	    size = file.tellg();
	    last_chunk_size=size;
	    //cout<<size<<"\n";
	    
		file.seekg (0, ios::beg);
	    
	    int chunk_count=size/chunk_size;
	    if(size % chunk_size != 0){
	    	chunk_count++;
	    	last_chunk_size=size % chunk_size;
	    }
	    for(int chnk=0;chnk<chunk_count;chnk++){
	    	streampos sz=(streampos) chunk_size;
	    	if(chnk == chunk_count-1){
	    		sz=last_chunk_size;
	    	}
	    	char * chunk;
	    	chunk = new char [sz];
		    file.read (chunk, sz);

		    unsigned char out[20];

		    SHA1((unsigned char*)chunk, strlen(chunk),out);
		    /*
		    for (int i = 0; i < 20; i++) {
		        printf("%02x", obuf[i]);
		    }
		    printf("\n");
		    */
		    std::stringstream s;
		        for (int i = 0; i < 10; ++i) 
		            s << hex << setfill('0') << setw(2) << (unsigned short) out[i];
		        //cout << s.str() << "\n";
		       string hash=s.str();
		       sha1_hash=sha1_hash+hash;

		    //printf("\n");
	    }
	    file.close();
	    return sha1_hash;
	}
	//cout << "the entire file content is in memory: \n";
	else{
		return "0";
	}
}

string get_file_name_from_path(string current){  // obtain name of file or directory from its absolute path
  string directory_name;
  int ind=current.length()-1;
  while(ind >=0 && current[ind]!='/'){
    ind--;
  }
  directory_name=current.substr(ind + 1);
  return directory_name;
}
/*
int main(int argc,const char *argv[]){
	string tracker1=argv[1];
	string tracker2=argv[2];
	string path=argv[3];
	string name=get_file_name_from_path(path);
	//cout<<name<<"\n";
	streampos file_size;

	ifstream file (path, ios::in|ios::binary|ios::ate);
	if (file.is_open()){
	    file_size = file.tellg();   
		file.seekg (0, ios::beg);
		file.close();
	}
	else{
		cout<<"file does not exist\n";
		return -1;
	}

	string hash=get_hash(path);

	ofstream torrent_file;
	string torrent_file_name=name+".mtorrent";
	torrent_file.open (torrent_file_name.c_str());

    torrent_file << tracker1 <<"\n";
    torrent_file << tracker2 <<"\n";
    torrent_file << name <<"\n";
    torrent_file << file_size <<"\n";
    torrent_file << hash <<"\n";
    torrent_file.close();
	//create_torrent_file(tracker1,tracker2,)
    //char ibuf[] = "compute sha1";
    
    return 0;
}

*/
/*
int main(int argc, const char *argv[]){
	string path=argv[1];
    //char ibuf[] = "compute sha1";
    streampos size;
	  char * memblock;
	  ifstream file (path, ios::in|ios::binary|ios::ate);
	  if (file.is_open())
	  {
	    size = file.tellg();
	    cout<<size<<"\n";
	    memblock = new char [size];
	    file.seekg (0, ios::beg);
	    file.read (memblock, size);
	    file.close();
	}
	cout << "the entire file content is in memory: \n";
    unsigned char obuf[20];

    //SHA1((unsigned char*)ibuf, strlen(ibuf),obuf);
    SHA1((unsigned char*)memblock, strlen(memblock),obuf);
    
    for (int i = 0; i < 20; i++) {
        printf("%02x", obuf[i]);
    }
    printf("\n");
    std::stringstream s;
        for (int i = 0; i < 20; ++i) 
            s << std::hex << std::setfill('0') << std::setw(2) << (unsigned short) obuf[i];
        std::cout << s.str() << std::endl;
       string hash=s.str();

    printf("\n");

    return 0;
}
*/