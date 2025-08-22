#pragma once
#include "Util.hpp"
#include "Vec.hpp"
#include "Image.hpp"
struct Mat{
    //static const int NAME_SIZE = 20;
    real em;
    Vec3 colour;
    //char name[NAME_SIZE];
    string name;
    Image image;

    Mat(){}
};