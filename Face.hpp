#pragma once
#include "Util.hpp"
#include "Vec.hpp"
#include "Mat.hpp"
struct Face{
    static const int VERT_COUNT = 3;
    bool isRectangle = false;
    Mat* mat;
    
    //Vec3 v0;
    //Vec3 v0v1;
    //Vec3 v0v2;
    Vec3 vertexList[VERT_COUNT];
    
    
    //Pre compute position to UV mapping if necessary
    Vec2 uvPosOnImage;
    union{
        Vec2 textureCoords[VERT_COUNT];
        Matrix3x3 toImageCoords;
    };    
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

    void PreComputeUVMappings(){
        if(mat == nullptr || mat->image.buffer == nullptr){
            return;
        }
        //Need to store for later use
        uvPosOnImage = textureCoords[0] * Vec2(mat->image.width, mat->image.height);
        Vec3 e1 = vertexList[1] - vertexList[0];
        Vec3 e2 = vertexList[2] - vertexList[0];
        Vec2 u0u1 = textureCoords[1] - textureCoords[0];
        Vec2 u0u2 = textureCoords[2] - textureCoords[0];
        Vec3 n = GetTrueNormal();
        Vec3 cols[3] = {e1,e2,n};
        Matrix3x3 m = Matrix3x3(cols).transpose();
        Matrix3x3 toFaceBasis = m.inverse();

        Vec3 cols2[3] = {
            Vec3(u0u1.x,u0u1.y,0),
            Vec3(u0u1.x,u0u1.y,0),
            Vec3(0,0,0)
        };
        Matrix3x3 toUVCoords = Matrix3x3(cols2).transpose();


        Vec3 rows[3] = {
            Vec3(mat->image.width,0,0),
            Vec3(0,mat->image.height,0),
            Vec3(0,0,0)
        };
        Matrix3x3 scaleToImgSize = Matrix3x3(rows);
        
        toImageCoords = (scaleToImgSize * (toUVCoords * toFaceBasis));
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