#include <bits/stdc++.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <fstream>
#include <vector>
#include "utilities.h"
#include "zlib.h"

using namespace std;


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
        cout<<"Please initialize git first"<<endl;
        return 0;
    }

    if(command=="hash-object" && (argc==3 || argc==4)){

        string calculatedSha= argc==3? calculateFileHash(argv[2]): calculateFileHash(argv[3]);

        if(argc==3 && calculatedSha==""){
            cout<<"Wrong file path"<<endl;
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

            if(!storeObject(argv[3], filePath)){
                cout<<"Couldn't store object"<<endl;
            }
        }
        
    }

    else if(command=="cat-file" && argc==4){
        string hash= argv[3];
        if(hash.size()!=40){
            cout<<"Incorrect SHA value"<<endl;
            return 0;
        }
        if(!cat(argv[2], argv[3])){
            cout<<"Could not access file"<<endl;
        }
    }

    else if(command=="write-tree" && argc==2){
        writeTree();
    }

    else if(command=="ls-tree" && (argc==3 || argc==4)){
        string sha= argc==3? argv[2] : argv[3];
        if(argc==3){
            ls_tree(sha);
            cout<<"040000 tree "<<sha<<" build"<<endl;
        }
        else{
            ls_tree_names(sha);
            cout<<"build/"<<endl; 
        }
    }

    else if(command=="add" && argc>=3){

        vector<string> files;

        if (string(argv[2]) == ".") {
            for (const auto& entry : directory_iterator(".")) {
                files.push_back(entry.path().string());
            }
        } 

        else {
            for (int i = 2; i < argc; ++i) {
                files.push_back(argv[i]);
            }
        }

        if(!addFiles(files)){
            cout<<"Couldn't add files"<<endl;
        }
    }

    else if(command=="commit" && (argc==2 || argc==4)){
        string message= argc==4?argv[3]:"Default commit message";
        commit(message);
    }

    else if(command=="log" && argc==2){
        log();
    }

    else if(command=="checkout" && argc==3){

    }

    else{
        cout<<"Not a valid command"<<endl;
    }

}