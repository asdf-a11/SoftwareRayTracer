#pragma once
#include "Util.hpp"
#include "Vec.hpp"
const int NAME_SIZE = 20;
struct Mat{
    real em;
    Vec3 colour;
    char name[NAME_SIZE];
};
struct Face{
    static const int VERT_COUNT = 3;
    bool isRectangle = false;
    Mat* mat;
    Vec3 vertexList[VERT_COUNT];
    Vec3 normal;

    Face(){}
    Face(Vec3 lst[3], Mat* mat){
        this->mat = mat;
        memcpy(vertexList, lst, sizeof(Vec3) * 3);
    }

    void SetNormal(){
        Vec3 i = vertexList[1] - vertexList[0];
        Vec3 j = vertexList[2] - vertexList[0];
        normal = cross(i,j).normalize();
    }
};
struct Object{
    Vec3 pos;
    vector<Face> faceList;
};