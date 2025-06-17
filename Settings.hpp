#pragma once
#include "Util.hpp"
typedef float real;

const int SPACE_CHUNCK_SPLIT = 2;
const int MAX_BOUNCES = 1;// 1 with 10 samples
const int SCREEN_WIDTH = 1920;//1300;
const int SCREEN_HEIGHT = 1080;//700;
const constexpr real FOV = deg2rad(90.0f);
const int SPACE_CHUNK_MAX_FACES = 6;
const constexpr real SPACE_CHUNK_DUPLICATE_FACE = 1.8f;
const int SAMPLES_FOR_BOUNCE_NUMBER[MAX_BOUNCES] = {
    500//50,10//10
};