#pragma once
#include "Vec.hpp"
struct Mat{
    static const int NAME_SIZE = 20;
    real em;
    Vec3 colour;
    char name[NAME_SIZE];
};