#pragma once
#include "Vec.hpp"
#include "Face.hpp"
struct Object{
    Vec3 pos;
    vector<Face> faceList;
};