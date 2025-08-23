#include <cstring>
#include <malloc.h>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <set>
#include "Settings.hpp"
#include "Util.hpp"
#include "Vec.hpp"
#include "Window.hpp"
#include "Matrix.hpp"
#include "Object.hpp"
#include "ObjectLoader.hpp"
#include "ReadBMP.hpp"


vector<vector<Vec3>> screenBuffer;
vector<vector<Vec3>> depthBuffer;
//Fixed arrays are usefull as the address of a element is gaurenteed to be constant 
//unlike vector 
FixedArray<Mat> worldMatList;
FixedArray<Face> worldFaceList;
vector<Face*> lightFaceList;

#include "SpaceChunk.hpp"
#include "DeNoiser.hpp"

struct Cam{
    Vec3 dir = Vec3(0.f);
    Vec3 pos = Vec3(0.f);
};

Cam cam;
SpaceChunk worldChunk;
vector<Object> objectList;


//faster than using sin on the FPU
real ApproxSin(real angle){
    real sign = (angle < 0.f) ? -1.f : 1.f;
    real normAngle = std::abs(angle) / (2.f * PI);
    normAngle -= std::floor(normAngle);
    real o = (normAngle < 0.5f) ? normAngle : -(normAngle-1.f);
    return 16.f * (normAngle-0.5f) * o * sign;
}
real ApproxCos(real angle){
    return ApproxSin(angle + PI / 2.f);
}
Vec3 GetRayDir(int x, int y, Vec3 camRot){
    // Normalized Device Coordinates (-1 to 1)
    real x_ndc = (real)x / (real)SCREEN_WIDTH * 2.f - 1.f;
    real y_ndc = 1.f - (real)y / (real)SCREEN_HEIGHT * 2.f;  // flip Y

    // Apply aspect ratio and FOV
    real aspect = (real)SCREEN_WIDTH / (real)SCREEN_HEIGHT;
    real fovScaler = std::tan(FOV * 0.5f);  // FOV in radians

    real x_camera = x_ndc * aspect * fovScaler;
    real y_camera = y_ndc * fovScaler;

    // Camera space ray
    Vec3 ray_camera = Vec3(-x_camera, y_camera, 1.0f);

    // Rotate to world space using camRot (assumes row-major matrix mul order)
    Vec3 ray_world = XRotationMatrix(camRot.x) * (YRotationMatrix(camRot.y) * (ZRotationMatrix(camRot.z) * ray_camera));

    return ray_world;
}
//spaceChunkList is returned when either empty or has the closes next hit leaf node
void GetNextHitSpaceChunk(Vec3 rayPos, Vec3 rayDir, vector<SpaceChunk*>* spaceChunkList){
    using std::pair;
    //1. Remove last node as it didn't hit anything in it
    spaceChunkList->pop_back();
    //2. Check hit last node
    while(spaceChunkList->size() != 0){
        SpaceChunk* last = (*spaceChunkList)[spaceChunkList->size()-1];
        if(last->hit(rayPos, rayDir)){          
            if(last->isLeaf){
                //x. Return when base level node is hit
                break;
            }
            else{
                //3. Expand hit node
                spaceChunkList->pop_back();
                //Allocate space on the stack
                int numberAdded = last->spaceChunkNumber;
                pair<SpaceChunk*, real>* buffer = (pair<SpaceChunk*, real>*)alloca(numberAdded * sizeof(pair<SpaceChunk*, real>));
                //pre-compute distances
                looph(i,numberAdded){
                    SpaceChunk* ptr = &last->lst[i];
                    buffer[i].first = ptr;
                    buffer[i].second = (ptr->pos - cam.pos).lengthSquared();
                }
                //sort using best algorithum for small numbers like 8
                std::sort(buffer, buffer + numberAdded, 
                    [](const auto& a, const auto& b) -> bool{
                        return a.second > b.second;
                    }
                );
                //add all entries onto the end of the vector          
                looph(i,numberAdded){
                    spaceChunkList->push_back(buffer[i].first);
                }                
            }
        }
        else{
            //4. Remove node if not hit
            spaceChunkList->pop_back();
        }
    }    
}
void PrintFaceList(Face* lst, int size){
    auto printVertex = [](Vec3 v){
        cout << "Vec3(" << v.x << ", " << v.y << ", " << v.z << ")";
    };
    looph(i,size){
        //printVertex(lst[i].vertexList[0]); cout << ", ";
        //printVertex(lst[i].vertexList[1]); cout << ", ";
        //printVertex(lst[i].vertexList[2]); cout << "\n";
    }
}
real RayFaceCollision(Vec3 rayPos, Vec3 rayDir, Face* facePtr){
    //V0 IS ALLWAYS UNIQUE VERTEX FOR WHEN isRectangle is true
    Vec3& v0 = facePtr->v0;
    //Vec3& v1 = facePtr->vertexList[1];
    //Vec3& v2 = facePtr->vertexList[2];
    Vec3 edge1 = facePtr->v0v1;
    Vec3 edge2 = facePtr->v0v2;
    Vec3 h = cross(rayDir, edge2);
    real a = dot(edge1, h);
    if (std::abs(a) < EPSILON) return -1.f; // Parallel

    real f = 1.0f / a;
    Vec3 s = rayPos - v0;
    real u = f * dot(s, h);
    if (u < 0.0f || u > 1.0f) return -1.f;

    Vec3 q = cross(s, edge1);
    real v = f * dot(rayDir, q);
    if(facePtr->isRectangle){
        if(v < 0.0f || v > 1.0f) return -1.f;
    }
    else{
        if (v < 0.0f || u + v > 1.0f) return -1.f;
    }

    real t = f * dot(edge2, q);
    return t;
}
Vec3 GetSkyColour(Vec3 rayDir){
    real a = dot(rayDir, Vec3(0.3,1,-0.3).normalize());
    return Vec3(0.9,0.9,1.f) * std::min(std::max(0.2f,a*2.f), 1.f) * 0.8f;
}
Vec3 GetReflectedRayDir(Vec3 incomingRayDir, Vec3 faceNormal, Face* facePtr, int rayNumber, int numberOfSamples){
    const real goldenRatio = (1.f + sqrtf(5.f)) / 2.f;
    real x = (real)rayNumber / goldenRatio;
    x = x - std::floor(x);
    real y = (real)rayNumber / (real)(numberOfSamples-1);
    real theta = 2.f * PI * x;
    real phi = std::acos(1.f-2.f*y);
    Vec3 sp = Vec3(
        ApproxCos(theta)*ApproxSin(phi),
        std::abs(1.f-2.f*y)+0.01f,
        ApproxSin(theta)*ApproxSin(phi)
    );
    Vec3 A = faceNormal;
    Vec3 B = (facePtr->v0v1).normalize();
    Vec3 C = cross(A,B).normalize();
    //sp Y must times by A to maintain up direction
    return Vec3(
        A[0] * sp[1] + B[0] * sp[0] + C[0] * sp[2],
        A[1] * sp[1] + B[1] * sp[0] + C[1] * sp[2],
        A[2] * sp[1] + B[2] * sp[0] + C[2] * sp[2]
    );
}
struct CastRay_Return{
    Vec3 hitNormal;
    Vec3 colour;
    real distance;
    Face* hitFacePtr;
};
//Returns colour and distance of collision
CastRay_Return CastRay(Vec3 rayPos, Vec3 rayDir, int bounceNumber,  Face* cantHitFace=nullptr)
{
    static vector<SpaceChunk*> spaceChunkList;
    spaceChunkList.clear();
    spaceChunkList.push_back(&worldChunk);
    spaceChunkList.push_back(nullptr);
    Vec3 colour;
    Vec3 hitNormal = FLOAT_MAX_VALUE;
    real minDistance = FLOAT_MAX_VALUE;
    Face* hitFacePtr = nullptr;
    Vec3 hitPosition;
    while(true){
        GetNextHitSpaceChunk(rayPos, rayDir, &spaceChunkList);
        if(spaceChunkList.size() == 0){break;}
        SpaceChunk* chunkToCheck = spaceChunkList[spaceChunkList.size()-1];
        //If the bounding box is further than nearist hit
        if((chunkToCheck->pos - rayPos).lengthSquared() - sq(chunkToCheck->size/2.f)> sq(minDistance * rayDir.lengthSquared())){
            break;
        }

        looph(faceCounter, chunkToCheck->faceNumber){
            Face* currentFacePtr = chunkToCheck->faceList[faceCounter];
            if(currentFacePtr == cantHitFace){continue;}
            //Face collision function called here
            real t = RayFaceCollision(rayPos, rayDir, currentFacePtr);
            if(t >= 0.f && t < minDistance){
                minDistance = t;
                //Bit weird computing hit here
                hitPosition = rayPos + rayDir * minDistance;
                hitFacePtr = currentFacePtr;
            }
        }

        //if(hitFacePtr != nullptr){
        //    break;
        //}
    }
    if(hitFacePtr != nullptr){
        #if true
        if(hitFacePtr->mat->image.buffer != nullptr){
            Vec3 hitRelPos = hitPosition - hitFacePtr->v0;
            Vec3 uvOffset = Vec3(hitFacePtr->uvPosOnImage.x,hitFacePtr->uvPosOnImage.y,0);
            Vec3 pixelCoords = uvOffset + (hitFacePtr->toImageCoords * hitRelPos);
            colour = hitFacePtr->mat->image.GetPixel(pixelCoords.x,pixelCoords.y);
        }
        else{
            colour = hitFacePtr->mat->colour;
        }        

        if(bounceNumber < MAX_BOUNCES && hitFacePtr->mat->em < 1.f){
            Vec3 avgOfColours = Vec3(0.f);
            const int SAMPLE_COUNT = SAMPLES_FOR_BOUNCE_NUMBER[bounceNumber];
            const int DIRECT_SAMPLE_COUNT = SAMPLE_COUNT * DIRECT_LIGHTING_PERCENT;
            const int STRATIFIED_SAMPLE_COUNT = SAMPLE_COUNT - DIRECT_SAMPLE_COUNT;
            Vec3 faceNormal = hitFacePtr->normal;
            if(dot(faceNormal, rayDir) > 0.f){
                faceNormal *= -1.f;
            }
            hitNormal = faceNormal;
            //Certain amount for direct light sampling
            looph(rayCounter, lightFaceList.size()){ //DIRECT_SAMPLE_COUNT
                static unsigned long long litFaceIndex = 0;
                Face* litFacePtr = lightFaceList[rayCounter];
                //Face* litFacePtr = lightFaceList[litFaceIndex%lightFaceList.size()];
                Vec3 middleOfFace = litFacePtr->v0 + litFacePtr->v0v1*0.5f + litFacePtr->v0v2*0.1f;
                Vec3 directionVector = (middleOfFace-hitPosition);
                real faceArea = litFacePtr->v0v1.length() * litFacePtr->v0v2.length() * 0.5f;
                //normalize ?
                litFaceIndex++;// = sq(litFaceIndex+1);
                //if(dot(faceNormal, directionVector) < 0.f){
                //    //rayCounter--;
                //    continue;
                //}
                //Vec3 newDir = GetReflectedRayDir(rayDir, faceNormal, hitFacePtr,  rayCounter, STRATIFIED_SAMPLE_COUNT);
                auto rayValues = CastRay(hitPosition, directionVector, MAX_BOUNCES, hitFacePtr);
                
                avgOfColours += (rayValues.colour) * faceArea / sq(rayValues.distance);
            }
            //Rest for stratified light sampling
            looph(rayCounter, STRATIFIED_SAMPLE_COUNT){
                Vec3 newDir = GetReflectedRayDir(rayDir, faceNormal, hitFacePtr,  rayCounter, STRATIFIED_SAMPLE_COUNT);
                auto rayValues = CastRay(hitPosition, newDir, MAX_BOUNCES, hitFacePtr);//bounceNumber+1
                avgOfColours += rayValues.colour;
            }
            //avgOfColours /= SAMPLE_COUNT;
            avgOfColours /= 2.f * PI;
            colour = colour * hitFacePtr->mat->em + colour * avgOfColours;
        }
        else{
            colour = colour * hitFacePtr->mat->em;
        }
        #else
        //Just return the colour of the face, good for debugging or testing
        //colour = hitFacePtr->mat->colour;
        colour = hitFacePtr->mat->image.GetPixel(pixelCoords.x,pixelCoords.y);
        #endif
    }
    else{
        colour = GetSkyColour(rayDir);
    }
    CastRay_Return crr;
    crr.colour = colour;
    crr.hitNormal = hitNormal;
    crr.distance = (rayDir * minDistance).length();
    crr.hitFacePtr = hitFacePtr;
    return crr;
}

void ExecuteRayTracer(int frameCounter){
    const int step = 5;
    int counter = 0;
    for(int x = frameCounter % step; x < SCREEN_WIDTH; x += step){
        for(int y = frameCounter % 2; y < SCREEN_HEIGHT; y += 2){
            Vec3 rayDir = GetRayDir(x,y,cam.dir);
            auto pixelData = CastRay(cam.pos, rayDir, 0);
            screenBuffer[x][y] = pixelData.colour;
            depthBuffer[x][y] = pixelData.hitNormal;

            if(counter % 10 == 0){
                cout << "dont percent " << (real)counter / (real)(SCREEN_HEIGHT*SCREEN_WIDTH)* 100.f << "\n";
            }
            counter ++;
        
        }
    }
    if(frameCounter % step == 0){
        //cam.pos += Vec3(0,0.05,0.1);
    }
}
void InitBuffers(){
    screenBuffer.resize(SCREEN_WIDTH);
    depthBuffer.resize(SCREEN_WIDTH);
    looph(i, SCREEN_WIDTH){
        screenBuffer[i].resize(SCREEN_HEIGHT);
        depthBuffer[i].resize(SCREEN_HEIGHT);
    }
}
void ClearScreenBuffer(){
    looph(i,screenBuffer.size()){
        looph(j,screenBuffer[i].size()){
            screenBuffer[i][j] = Vec3(0.f, 0.f, 0.f);
        }
    }
}
void DrawScreenBuffer(Graphics::Window* window){
    looph(x,SCREEN_WIDTH){
        looph(y, SCREEN_HEIGHT){
            window->DrawPixel(x, y, screenBuffer[x][y]);
        }
    }
}
void GenerateWorldFaceList(vector<Object>& objList){
    int totalFaceNumber = 0;
    looph(i,objList.size()){
        totalFaceNumber += objList[i].faceList.size();
    }
    worldFaceList.AllocArray(totalFaceNumber);
    int counter = 0;
    looph(i,objList.size()){
        looph(f, objList[i].faceList.size()){
            Face nf = objList[i].faceList[f];
            nf.v0 += objList[i].pos;
            nf.SetNormal();
            nf.isRectangle = false;
            worldFaceList[counter++] = nf;
        }
    }
}
vector<Face*> GetWorldFacePtrList(){
    vector<Face*> out(worldFaceList.size());
    looph(i,worldFaceList.size()){
        out[i] = &worldFaceList[i];
    }
    return out;
}
void FillEmmisiveFaceList(){
    lightFaceList = {};
    looph(i,worldFaceList.size()){
        if(worldFaceList[i].mat->em > 0.f){
            lightFaceList.push_back(&(worldFaceList[i]));
        }
    }
    cout << "Number of light faces " << lightFaceList.size() << "\n"; 
}
void PreComputeAllFacesUVMappings(){
    looph(i,worldFaceList.size()){
        worldFaceList[i].PreComputeUVMappings();
    }
}
void LoadEverything(string objPath){
    //string filePath = "Models/uploads_files_3581871_LION_STATUE_obj/me.obj";//"Models/uploads_files_3825299_Low+poly+bedroom_Obj/triModel.obj";
    objectList = ReadMeshFile(objPath, &worldMatList);
    //Set position of entire object
    objectList[objectList.size()-1].pos = Vec3(-2,-2,5);

    GenerateWorldFaceList(objectList);
    vector<Face*> ptrList = GetWorldFacePtrList();
    worldChunk.SetSizeAndPos();
    worldChunk.Init(&ptrList);
    #if true
    worldChunk.RemoveNodesWithSingleChild();
    vector<Face*> facesToRemove;
    worldChunk.Create4VertFaces(&facesToRemove);
    cout << "Number of faces removed -> " << facesToRemove.size() << "\n";
    cout << "Number of faces " << worldFaceList.size() << "\n";
    //Remove all
    worldChunk.RemoveDudFaces(&facesToRemove);
    //worldChunk.PrintInfo();
    #endif
    FillEmmisiveFaceList();
    PreComputeAllFacesUVMappings();
}
int main(){
    using namespace Graphics;
    InitBuffers();

    #if false
            //Position used for testing test scene
        cam.pos = Vec3(-4,2,5);
        cam.dir = Vec3(0,deg2rad(100.f),0);
    #else
        //cam.pos = Vec3(0.38f, 1.f, 9.17f);
        //cam.dir = Vec3(0,3.63f, 0);
        //cam.pos = Vec3(-6.51, 2, 8.59);
        cam.dir = Vec3(0, -6.48365, 0);
        cam.pos = Vec3(-1.5872, 10, -0.974691);
    #endif

    LoadEverything("Models/armoury/blenderLighting/LightingModel.obj");
    #if true
    Window window(SCREEN_WIDTH,SCREEN_HEIGHT,"Raytracer");
    window.Init();
    //worldChunk.PrintInfo();

    window.StartLoop([](Graphics::Window* window){
        cout << "camPos=" << cam.pos << " camDir=" << cam.dir << "\n";
        #if false
            if(window->frameCounter <= 10)
                ExecuteRayTracer(window->frameCounter);
            else{
                DeNoiser dn;
                looph(i,1){
                    dn.DeNoise(screenBuffer, depthBuffer);
                }
                DrawScreenBuffer(window);
                cout << "Done\n";
                for(;;){}
            }
        #else
            const real speed = 0.025f * 0.25f;
            Vec3 changeDir = Vec3(0.f);
            if(true){
                changeDir += Vec3(window->IsKeyPressed(XK_a) - window->IsKeyPressed(XK_d),window->IsKeyPressed(XK_v) - window->IsKeyPressed(XK_space),window->IsKeyPressed(XK_w) - window->IsKeyPressed(XK_s)) * speed;
            }
            Vec3 oldCamDir = cam.dir;

            if(changeDir != Vec3(0.f) || cam.dir != oldCamDir){
                ClearScreenBuffer();
                window->frameCounter = 0;
                changeDir = changeDir.normalize();
                cam.pos += XRotationMatrix(cam.dir.x) * (YRotationMatrix(cam.dir.y) * (ZRotationMatrix(cam.dir.z) * changeDir));
            }
            cam.dir += Vec3(0,deg2rad(-window->IsKeyPressed(XK_e) + window->IsKeyPressed(XK_q)),0) * 4.f;
            ExecuteRayTracer(window->frameCounter);
        #endif
        DrawScreenBuffer(window);
    });
    #else
    looph(frameCounter, 1){
        looph(i,5*2){
            ExecuteRayTracer(i);
        }
    }
    #endif
}
