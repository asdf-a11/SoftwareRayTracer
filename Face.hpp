#pragma once
#include "Util.hpp"
#include "Vec.hpp"
#include "Mat.hpp"
struct Face{
    static const int VERT_COUNT = 3;
    bool isRectangle = false;
    Mat* mat;
    Vec3 vertexList[VERT_COUNT];
    Vec3 normal;

    Face(){}
    Face(Vec3 lst[3], Mat* mat){
        this->mat = mat;
        looph(i,3){
            vertexList[i] = lst[i];
        }
    }

    void SetNormal(){
        Vec3 i = vertexList[1] - vertexList[0];
        Vec3 j = vertexList[2] - vertexList[0];
        normal = cross(i,j).normalize();
    }
};