#include <bits/stdc++.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <fstream>
#include <vector>

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
    bool isInitialized= false;

    if(argc<2){
        cout<<"Invalid input"<<endl;
        return 0;
    }
    
    string command= argv[1];


    if(command=="init" && argc==2){
        initializeRepo();
        isInitialized= true;
    }

    else if(isInitialized){

        if(command=="hash-object" && (argc==3 || argc==4)){

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

    else{
        cout<<"Please first initialize the repository"<<endl;
    }


}