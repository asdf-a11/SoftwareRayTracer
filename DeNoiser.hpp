#pragma once
#include "Vec.hpp"
#include "Util.hpp"
#include "Settings.hpp"
struct DeNoiser{
    Vec3 DeNoisePixel(vector<vector<Vec3>>& screenBuffer, vector<vector<Vec3>>& depthBuffer, int x, int y){
        Vec3 currentColour = screenBuffer[x][y];
        Vec3 currentNormal = depthBuffer[x][y];
        Vec3 avgColour = 0;
        int pixelsToAverage = 0;
        loop(xOffset,-1,1){
            loop(yOffset,-1,1){
                if(xOffset == 0 && yOffset == 0){
                    continue;
                }
                Vec3 colour = screenBuffer[x+xOffset][y+yOffset];
                Vec3 normal = depthBuffer[x+xOffset][y+yOffset];
                if(currentNormal.x != FLOAT_MAX_VALUE && normal.x != FLOAT_MAX_VALUE && dot(normal, currentNormal) >= 0.9f){
                    avgColour += colour;
                    pixelsToAverage++;
                }
            }
        }
        avgColour += currentColour;
        avgColour /= pixelsToAverage + 1;
        return avgColour;
    }
    void DeNoise(vector<vector<Vec3>>& screenBuffer, vector<vector<Vec3>>& depthBuffer){
        vector<vector<Vec3>> newBuffer = screenBuffer;
        //looph(i,SCREEN_WIDTH){
        //    newBuffer[i].resize(SCREEN_HEIGHT);
        //}

        //Simplfies the code a bit to not bother with edge values
        loop(x,1,SCREEN_WIDTH-1){
            loop(y,1,SCREEN_HEIGHT-1){
                newBuffer[x][y] = DeNoisePixel(screenBuffer, depthBuffer, x, y);
            }
        }

        //Update buffer to new buffer
        screenBuffer = newBuffer;
    }
};