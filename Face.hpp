#pragma once
#include "Util.hpp"
#include "Vec.hpp"
#include "Mat.hpp"
struct Face{
    static const int VERT_COUNT = 3;
    bool isRectangle = false;
    Mat* mat;
    Vec3 vertexList[VERT_COUNT];
    //Pre compute position to UV mapping if necessary
    Vec2 textureCoords[VERT_COUNT];
    Vec3 normal;

    Face(){}
    Face(Vec3 lst[3], Mat* mat, Vec3 faceNormal, Vec2 textureCoords[3]){
        this->mat = mat;
        normal = faceNormal;
        looph(i,VERT_COUNT){
            vertexList[i] = lst[i];
            this->textureCoords[i] = textureCoords[i];
        }
    }

    Vec3 GetTrueNormal(){
        Vec3 i = vertexList[1] - vertexList[0];
        Vec3 j = vertexList[2] - vertexList[0];
        return cross(i,j).normalize();
    }
    //Sometimes a obj file comes with normals, else calculate
    //them with this function
    void SetNormal(){
        normal = GetTrueNormal();
    }
};