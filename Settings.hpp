#pragma once
#include "Util.hpp"
typedef float real;

const int SPACE_CHUNCK_SPLIT = 2;
const int MAX_BOUNCES = 1;
const int SCREEN_WIDTH = 1300;
const int SCREEN_HEIGHT = 700;
const constexpr real FOV = deg2rad(90.0f);
const constexpr real SPACE_CHUNK_MIN_SIZE = 1.1f;
const int SAMPLES_FOR_BOUNCE_NUMBER[] = {
    10
};