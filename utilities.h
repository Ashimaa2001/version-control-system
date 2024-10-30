#ifndef UTILITIES_H
#define UTILITIES_H

#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <openssl/sha.h>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <zlib.h>
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

bool isGitInitialized(){
    ifstream file;
    file.open(".mygit");
    if(!file){
        return false;
    }
    file.close();
    return true;
}

string calculateFileHash(const string& filePath) {

    ifstream file(filePath, ios::binary);

    if(!file){
        return "";
    }

    string fileContent((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    unsigned char hashValue[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(fileContent.c_str()), fileContent.size(), hashValue);
    
    stringstream shaValue;
    
    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
        shaValue << hex << setw(2) << setfill('0') << static_cast<int>(hashValue[i]);
    }
    return shaValue.str();
}

bool writeToFile(const string& filePath, const string& data){
    ofstream destFile(filePath, ios::binary);
    
    if(!destFile.is_open()){
        return false;
    }

    destFile.write(data.c_str(), data.size());
    destFile.close();
    return true;
}

bool storeObject(const string& filePath, const string& objectPath){

    ifstream file(filePath, ios::binary);
    if(!file){
        return false;
    }

    stringstream ss;
    ss<<file.rdbuf();
    string data= ss.str();

    uLongf compressedSize = compressBound(data.size());
    string compressedData(compressedSize, '\0');

    int result = compress(reinterpret_cast<Bytef*>(&compressedData[0]), &compressedSize,
                            reinterpret_cast<const Bytef*>(data.data()), data.size());

    if (result != Z_OK) {
        return false;
    }

    compressedData.resize(compressedSize);

    return writeToFile(objectPath, compressedData);
    
}

bool cat(const string& flag, const string& fileSha){

    string objectPath= ".mygit/objects/" + fileSha.substr(0, 2) + "/" + fileSha.substr(2);

    int fd = open(objectPath.c_str(), O_RDONLY);
    if(fd==-1){
        return false;
    }

    if(flag=="-t"){
        cout<<"blob"<<endl;
        return true;
    }

    off_t size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    string compressedData(size, '\0');
    read(fd, &compressedData[0], size);

    close(fd);

    uLongf decompressedSize = compressedData.size() * 5;
    string decompressedData(decompressedSize, '\0');

    int result = uncompress(reinterpret_cast<Bytef*>(&decompressedData[0]), &decompressedSize,
                              reinterpret_cast<const Bytef*>(compressedData.data()), compressedData.size());

    if (result != Z_OK) {
        return false;
    }

    decompressedData.resize(decompressedSize);

    if(flag=="-s"){
        cout<<decompressedData.size()<<endl;
    }
    else if(flag=="-p"){
        cout<<decompressedData<<endl;
    }
    return true;
}

#endif