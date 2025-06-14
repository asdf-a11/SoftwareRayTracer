#pragma once
#include <iostream>
#include <string>
#include <vector>

#ifndef SAFE
    #define SAFE false
#endif

#ifdef _WIN32
#define DIR_SEPERATOR '\\'
#else
#define DIR_SEPERATOR '/'
#endif

#define null 0
#define EPSILON 1e-5f //1e-6f
#define numberof(x) (sizeof(x)/sizeof(x[0]))
#define PI 3.141592653589793f
#define FLOAT_MAX_VALUE (3.4E+37)
#define deg2rad(x) ((x)/360.f*2.f*PI)
#define fnptr(returnType, name, ...) returnType (*name)(__VA_ARGS__)
#define sq(x) ((x) * (x))
#define pow3(x) ((x) * sq(x))
#define loop(i,l,h) for(int i = (l); i < (h); ++i)
#define loopr(i,l,h) for(int i = (l); i >= (h); --i)
#define looph(i,n) for(int i = 0; i < (n); ++i)
#define loopv(v,i,l,h) for(v i = (l); i < (h); ++i)
#define loopvh(v,i,h) for(v i = 0; i < (h); ++i)

#define uint unsigned int
#define byte unsigned char
#define uchar byte
#define ulong unsigned long


using std::cout;
using std::string;
using std::vector;
using std::cerr;

template<class T>
class FixedArray{
public:
    T* lst
        #if SAFE == true
            = nullptr
        #endif
    ;
    uint size_priv = 0;
public:
    FixedArray(){}
    FixedArray(uint size){
        this->size_priv = size;
        AllocArray(size);
    }
    void AllocArray(uint size){
        #if SAFE == true
            if(lst != nullptr){
                cout << "Error\n";
                return;
            }
        #endif
        lst = (T*)malloc(size * sizeof(T));
        size_priv = size;
    }
    T& operator[] (uint index){
        #if SAFE == true
            if(index >= size_priv){
                cout << "Index out of range\n";
            }
        #endif
        return lst[index];
    }
    T* ptr(){
        return lst;
    }
    uint size(){
        return size_priv;
    }
    ~FixedArray(){
        if(lst!=nullptr){
            free(lst);
        }        
    }
};
template<class T>
bool in(vector<T>& lst, T value){
    looph(i,lst.size()){
        if(lst[i] == value){
            return true;
        }
    }
    return false;
}