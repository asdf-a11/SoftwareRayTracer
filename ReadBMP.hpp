#pragma once
#include <fstream>
#include "Util.hpp"
#include "Image.hpp"

//Function found on stack overflow, slightly modified
Image ReadBMP(char* filename){
    int i;
    FILE* f = fopen(filename, "rb");

    if(f == null)
        throw "Argument Exception";

    unsigned char info[54];
    fread(info, sizeof(unsigned char), 54, f); // read the 54-byte header

    // extract image height and width from header
    int width = *(int*)&info[18];
    int height = *(int*)&info[22];

    
    cout << "  Name: " << filename << "\n";
    cout << " Width: " << width << "\n";
    cout << "Height: " << height << "\n";

    int row_padded = (width*3 + 3) & (~3);
    unsigned char* data = new unsigned char[row_padded];
    byte* image = new byte[width*height*3*sizeof(byte)];
    unsigned char tmp;

    int indexCounter = 0;
    for(int i = 0; i < height; i++)
    {
        fread(data, sizeof(unsigned char), row_padded, f);
        for(int j = 0; j < width*3; j += 3)
        {
            // Convert (B, G, R) to (R, G, B)
            tmp = data[j];
            data[j] = data[j+2];
            data[j+2] = tmp;

            //cout << "R: "<< (int)data[j] << " G: " << (int)data[j+1]<< " B: " << (int)data[j+2]<< endl;
        }
        for(int j = 0; j < width*3; j++){
            image[indexCounter] = data[j];
            indexCounter++;
        }
    }

    fclose(f);
    delete[] data;
    Image imageObj(width, height, image);
    return imageObj;
}