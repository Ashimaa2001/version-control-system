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

bool isGitInitialized(){
    ifstream file;
    file.open(".mygit");
    if(!file){
        cout<<"Please initialize git first"<<endl;
        return false;
    }
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


bool compressFile(const string& filePath, const string& destPath) {
    ifstream sourceFile(filePath, ios::binary);
    if (!sourceFile.is_open()) {
        return false;
    }

    string buffer((istreambuf_iterator<char>(sourceFile)), istreambuf_iterator<char>());
    sourceFile.close();

    string header = "blob " + to_string(buffer.size()) + "\0";
    string combinedData = header + buffer;

    uLongf compressedSize = compressBound(combinedData.size());
    string compressedData(compressedSize, '\0');

    if (compress(reinterpret_cast<Bytef*>(&compressedData[0]), &compressedSize,
             reinterpret_cast<const Bytef*>(combinedData.data()), combinedData.size()) != Z_OK) {
        return false;
    }

    compressedData.resize(compressedSize);

    ofstream destFile(destPath, ios::binary);
    
    destFile.is_open();
    
    destFile.write(compressedData.data(), compressedSize);
    destFile.close();

    return true;
}



#endif