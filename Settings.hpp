#pragma once
#include "Util.hpp"
typedef float real;

const int SPACE_CHUNCK_SPLIT = 2;
const int MAX_BOUNCES = 1;// 1 with 10 samples
const int SCREEN_WIDTH = 600;//1920;//1300;
const int SCREEN_HEIGHT = 600;//1080;//700;
const constexpr real FOV = deg2rad(90.0f);
const int SPACE_CHUNK_MAX_FACES = 6;
const constexpr real SPACE_CHUNK_DUPLICATE_FACE = 1.8f;
const constexpr real DIRECT_LIGHTING_PERCENT = 0.0f;
const int SAMPLES_FOR_BOUNCE_NUMBER[MAX_BOUNCES] = {
    30//500//50,10
};