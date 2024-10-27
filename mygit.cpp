#include <bits/stdc++.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <fstream>
#include <vector>
#include "utilities.h"
#include "zlib.h"

using namespace std;


void initializeRepo(){
    vector<const char*> dirs= {".mygit", ".mygit/objects", ".mygit/refs", ".mygit/info"};
    vector<const char*> files= {".mygit/config", ".mygit/description", ".mygit/HEAD"};
    ofstream file;

    for(const char* dir: dirs){
        mkdir(dir, 0775);
    }

    for(const char* filePath: files){
        file.open(filePath);
        file.close();
    }

}


int main(int argc, char *argv[]){

    if(argc<2){
        cout<<"Invalid input"<<endl;
        return 0;
    }
    
    string command= argv[1];

    if(command=="init" && argc==2){
        initializeRepo();
        return 0;
    }

    if(!isGitInitialized()){
        return 0;
    }

    if(command=="hash-object" && (argc==3 || argc==4)){

        string calculatedSha= argc==3? calculateFileHash(argv[2]): calculateFileHash(argv[3]);

        if(argc==3 && calculatedSha==""){
            cout<<"Could not locate file"<<endl;
            return 0;
        }
        
        cout<<calculatedSha<<endl;

        if(argc==4 && (string)argv[2]=="-w"){
        
            string dir= ".mygit/objects/"+calculatedSha.substr(0,2);
            string filePath= dir+ "/"+ calculatedSha.substr(2);
            ofstream file;

            mkdir(dir.c_str(), 0775);

            file.open(filePath);
            file.close();

            compressFile(argv[3], filePath);
        }
        
    }

    else if(command=="cat-file" && argc==4){

    }

    else if(command=="write-tree" && argc==2){

    }

    else if(command=="ls-tree" && (argc==3 || argc==4)){

    }

    else if(command=="add" && argc>=3){

    }

    else if(command=="commit" && (argc==2 || argc==4)){

    }

    else if(command=="log" && argc==2){

    }

    else if(command=="checkout" && argc==3){

    }

    else{
        cout<<"Not a valid command"<<endl;
    }

}