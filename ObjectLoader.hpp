#pragma once
#include <string>
#include <cstring>
#include <fstream>
#include "Util.hpp"
#include "Object.hpp"
#include "Face.hpp"
#include "Mat.hpp"

using std::string;

string FileToString(string fileName){
    std::ifstream t(fileName);
    t.seekg(0, std::ios::end);
    size_t size = t.tellg();
    std::string buffer(size, ' ');
    t.seekg(0);
    t.read(&buffer[0], size); 
    return buffer;
}
vector<string> SplitString(string content, char splitter='\n'){
    string c;
    vector<string> out;
    looph(i,content.size()){
        char v = content[i];
        if(v == splitter){
            if(c.size() != 0)out.push_back(c);
            c = "";
        }else{
            c.push_back(content[i]);
        }
    }
    if(c.size() != 0){
        out.push_back(c);
    }
    return out;
}
void FillWorldMatList(string fileName, FixedArray<Mat>* worldMatList){
    using std::stof;
    std::ifstream file(fileName);
    if(file.is_open() == false){
        cerr << "\n\nCouldnt open the file for mats " << fileName << "!!!\n\n";
        exit(EXIT_FAILURE);
    }
    vector<Mat> matList = {Mat()};
    //Default mat for shapes with no mat
    matList[matList.size()-1].colour = Vec3(0.7,0.3,0.3);
    memcpy(matList[matList.size()-1].name, "None", sizeof("None"));
    string line;
    while(getline(file, line)){
        vector<string> wordList = SplitString(line, ' ');
        if(wordList.size() == 0){continue;}
        string lineMeaning = wordList[0];
        if(lineMeaning == "newmtl"){
            Mat m;
            m.em = 0.f;
            memset(m.name, null, sizeof(m.name));
            memcpy(m.name, wordList[1].c_str(), std::min(m.NAME_SIZE-1, (int)wordList[1].size()) * sizeof(char));
            //m.name[NAME_SIZE-1] = null;
            matList.push_back(m);
        }
        else if(lineMeaning == "Kd"){
            matList[matList.size()-1].colour = Vec3(stof(wordList[1]), stof(wordList[2]), stof(wordList[3]));
        }
        else if(lineMeaning == "Ke"){
            Vec3 c = Vec3(stof(wordList[1]), stof(wordList[2]), stof(wordList[3]));
            if(c.x + c.y + c.z > EPSILON*3.f){
                matList[matList.size()-1].colour = c;
                matList[matList.size()-1].em = 100.f;
            }  
        }
    }
    int size = matList.size();
    worldMatList->AllocArray(size);
    looph(i,size){
        (*worldMatList)[i] = matList[i];
    }
}
vector<Object> ReadMeshFile(string fileName, FixedArray<Mat>* worldMatList){
    vector<Object> objList;
    //string contents = FileToString(fileName);
    std::ifstream file(fileName);
    if(file.is_open() == false){
        cerr << "\n\nCouldnt open the file " << fileName << "!!!\n\n";
        exit(EXIT_FAILURE);
    }
    vector<Vec3> currentObjVertList;
    string line;
    int currentMatIndex = 0;
    while(getline(file,line)){
        vector<string> wordList = SplitString(line, ' ');
        string lineMeaning = wordList[0];
        if(lineMeaning == "v"){
            currentObjVertList.push_back(Vec3(
             std::stof(wordList[1]),
             std::stof(wordList[2]),
             std::stof(wordList[3])  
            ));
        }else if(lineMeaning == "o"){
            objList.push_back(Object());
            currentObjVertList = {};
        }
        else if(lineMeaning == "mtllib"){
            string newFileName = fileName;
            if(newFileName.size() == 0){
                cerr << "Invalid material file name!!\n";
                exit(EXIT_FAILURE);
            }
            while(newFileName[newFileName.size()-1] != DIR_SEPERATOR){
                newFileName.pop_back();
            }
            newFileName += wordList[1];
            FillWorldMatList(newFileName, worldMatList);
        }
        else if (lineMeaning == "usemtl"){
            string n = wordList[1];
            int index = -1;
            looph(i,worldMatList->size()){
                string name = string((*worldMatList)[i].name);
                if(name == n){
                    index = i;
                    break;
                }
            }
            if(index == -1){
                cerr << "\nCould find mat!!\n"; 
                exit(EXIT_FAILURE);
            }
            currentMatIndex = index;
        }
        else if(lineMeaning == "f"){
            int vertIndex[3] = {
                std::stoi(wordList[1])-1,
                std::stoi(wordList[2])-1,
                std::stoi(wordList[3])-1,
            };
            looph(i,3){
                if(vertIndex[i] >= currentObjVertList.size() || vertIndex[i] < 0){
                    cerr << "Invalid vert number\n";
                    exit(EXIT_FAILURE);
                }
            }
            Vec3 vertList[3] = {
                currentObjVertList[vertIndex[0]],
                currentObjVertList[vertIndex[1]],
                currentObjVertList[vertIndex[2]]
            };
            objList[objList.size()-1].faceList.push_back(Face(
                vertList, &(*worldMatList)[currentMatIndex]
            ));
        }
    }
    file.close();
    return objList;
}