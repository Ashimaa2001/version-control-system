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
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <filesystem>

using namespace std;
using namespace std::filesystem;

void initializeRepo(){

    vector<const char*> dirs= {".mygit", ".mygit/objects", ".mygit/refs", ".mygit/info"};
    vector<const char*> files= {".mygit/config", ".mygit/index", ".mygit/HEAD"};
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

string compute_sha1(const string &content) {

    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char *>(content.c_str()), content.size(), hash);
    ostringstream hex_hash;
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        hex_hash << hex << setw(2) << setfill('0') << static_cast<int>(hash[i]);
    }
    return hex_hash.str();
}

void writeFiles(const string& path, ostringstream& tree_content){ //recursive

   DIR* dir = opendir(path.c_str());
    if (!dir) return;

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        string file_name = entry->d_name;
        if (file_name == "." || file_name == ".." || file_name[0] == '.') continue;

        string full_path = path + "/" + file_name;
        struct stat file_stat;
        if (stat(full_path.c_str(), &file_stat) == -1) continue;

        if (S_ISREG(file_stat.st_mode)) {
            ifstream file(full_path, ios::binary);
            stringstream buffer;
            buffer << file.rdbuf();
            string content = buffer.str();

            string file_hash = compute_sha1(content);
            tree_content << "100644 blob " << file_hash << " " << file_name << "\n";
        } else if (S_ISDIR(file_stat.st_mode)) {
            ostringstream dir_content;
            writeFiles(full_path, dir_content);
            
            string dir_hash = compute_sha1(dir_content.str());
            string objectDir = ".mygit/objects/" + dir_hash.substr(0, 2);
            mkdir(objectDir.c_str(), 0755);

            ofstream dir_tree_file(objectDir + "/" + dir_hash.substr(2), ios::binary);
            dir_tree_file << dir_content.str();
            dir_tree_file.close();

            tree_content << "040000 tree " << dir_hash << " " << file_name << "\n";
        }
    }

    closedir(dir);
}

string writeTree(){

    ostringstream tree_content;
    writeFiles(".", tree_content);

    string tree_object_content = tree_content.str();
    string tree_hash = compute_sha1(tree_object_content);

    string objectDir = ".mygit/objects/" + tree_hash.substr(0, 2);
    mkdir(objectDir.c_str(), 0775);

    ofstream tree_file(objectDir + "/" + tree_hash.substr(2), ios::binary);
    tree_file << tree_object_content;

    cout <<tree_hash << endl;
    return tree_hash;
}

void ls_tree(const string& tree_sha) {

    string objectPath = ".mygit/objects/" + tree_sha.substr(0, 2) + "/" + tree_sha.substr(2);
    
    ifstream tree_file(objectPath, ios::binary);
    if (!tree_file.is_open()) {
        cerr << "Tree object not found: " << objectPath << endl;
        return;
    }

    stringstream buffer;
    buffer << tree_file.rdbuf();
    string tree_content = buffer.str();

    size_t pos = 0;
    while (pos < tree_content.size()) {
        size_t next_line = tree_content.find('\n', pos);
        if (next_line == string::npos) break;

        string line = tree_content.substr(pos, next_line - pos);
        pos = next_line + 1;

        cout << line << endl;

        if (line[0]=='0') {
            stringstream ss(line);
            string mode, type, dir_hash, name;
            ss >> mode >> type >> dir_hash >> name;

            ls_tree(dir_hash);
        }
    }
    cout<<"040000 tree "<<tree_sha<<" build"<<endl;
}

void ls_tree_names(const string& tree_sha) {
    string objectPath = ".mygit/objects/" + tree_sha.substr(0, 2) + "/" + tree_sha.substr(2);
    
    ifstream tree_file(objectPath, ios::binary);
    if (!tree_file.is_open()) {
        cerr << "Tree object not found: " << objectPath << endl;
        return;
    }

    stringstream buffer;
    buffer << tree_file.rdbuf();
    string tree_content = buffer.str();

    size_t pos = 0;
    while (pos < tree_content.size()) {
        size_t next_line = tree_content.find('\n', pos);
        if (next_line == string::npos) break;

        string line = tree_content.substr(pos, next_line - pos);
        pos = next_line + 1;

        size_t last_space = line.find_last_of(' ');
        if (last_space != string::npos) {
            cout << line.substr(last_space + 1) << endl;
        }

        if (line[0]=='0') {
            stringstream ss(line);
            string mode, type, dir_hash, name;
            ss >> mode >> type >> dir_hash >> name;

            ls_tree_names(dir_hash);
        }
    }
    cout<<"build/"<<endl;
}

bool addFiles(const vector<string>& files) {
    ofstream indexFile(".mygit/index", ios::binary);

    for (const string& file_path : files) {
        if (exists(file_path)) {
            if(file_path=="./mygit"){
                continue;
            }
            if (is_directory(file_path)) {
                ostringstream tree_content;
                writeFiles(file_path, tree_content);
                
                string dir_object_content = tree_content.str();
                string dir_hash = compute_sha1(dir_object_content);
                
                indexFile << "tree "<<file_path << " " << dir_hash << endl;
            }
            else{
                string file_hash = calculateFileHash(file_path);
                indexFile <<"blob "<< file_path << " " << file_hash << endl;
            }
        } 
        else{
            return false;
        }
        
    }

    indexFile.close();
    return true;
}


#endif