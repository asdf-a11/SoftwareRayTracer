#include <cstring>
#include <malloc.h>
#include <iostream>
#include <algorithm>
#include <set>
#include "Settings.hpp"
#include "Util.hpp"
#include "Vec.hpp"
#include "Window.hpp"
#include "Matrix.hpp"
#include "Object.hpp"
#include "ObjectLoader.hpp"


vector<vector<Vec3>> screenBuffer;
FixedArray<Mat> worldMatList;
FixedArray<Face> worldFaceList;

#include "SpaceChunk.hpp"

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
        printVertex(lst[i].vertexList[0]); cout << ", ";
        printVertex(lst[i].vertexList[1]); cout << ", ";
        printVertex(lst[i].vertexList[2]); cout << "\n";
    }
}
real RayFaceCollision(Vec3 rayPos, Vec3 rayDir, Face* facePtr){
    //V0 IS ALLWAYS UNIQUE VERTEX FOR WHEN isRectangle is true
    Vec3& v0 = facePtr->vertexList[0];
    Vec3& v1 = facePtr->vertexList[1];
    Vec3& v2 = facePtr->vertexList[2];
    Vec3 edge1 = v1 - v0;
    Vec3 edge2 = v2 - v0;
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
    Vec3 B = (facePtr->vertexList[1] - facePtr->vertexList[0]).normalize();
    Vec3 C = cross(A,B).normalize();
    //sp Y must times by A to maintain up direction
    return Vec3(
        A[0] * sp[1] + B[0] * sp[0] + C[0] * sp[2],
        A[1] * sp[1] + B[1] * sp[0] + C[1] * sp[2],
        A[2] * sp[1] + B[2] * sp[0] + C[2] * sp[2]
    );
}
Vec3 CastRay(Vec3 rayPos, Vec3 rayDir, int bounceNumber,  Face* cantHitFace=nullptr)
{
    static vector<SpaceChunk*> spaceChunkList;
    spaceChunkList.clear();
    spaceChunkList.push_back(&worldChunk);
    spaceChunkList.push_back(nullptr);
    Vec3 colour;
    real minDistance = FLOAT_MAX_VALUE;
    Face* hitFacePtr = nullptr;
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
                hitFacePtr = currentFacePtr;
            }
        }
    }
    if(hitFacePtr != nullptr){
        #if true
        colour = hitFacePtr->mat->colour * hitFacePtr->mat->em;
        if(bounceNumber < MAX_BOUNCES && hitFacePtr->mat->em < 1.f){
            Vec3 avgOfColours = Vec3(0.f);
            const int SAMPLE_COUNT = SAMPLES_FOR_BOUNCE_NUMBER[bounceNumber];
            Vec3 faceNormal = hitFacePtr->normal;
            if(dot(faceNormal, rayDir) > 0.f){
                faceNormal *= -1.f;
            }
            looph(rayCounter, SAMPLE_COUNT){
                Vec3 newDir = GetReflectedRayDir(rayDir, faceNormal, hitFacePtr,  rayCounter, SAMPLE_COUNT);
                avgOfColours += CastRay(rayDir*minDistance + rayPos, newDir, bounceNumber+1, hitFacePtr);
            }
            avgOfColours /= SAMPLE_COUNT;
            colour += hitFacePtr->mat->colour * avgOfColours;
        }
        #else
        union{
            SpaceChunk* ptr;
            byte cList[3];
        };
        ptr = chunkToCheck;
        colour = Vec3(cList[0], cList[1], cList[2]) / 255.f;
        #endif
    }
    else{
        colour = GetSkyColour(rayDir);
    }
    return colour;
}

void ExecuteRayTracer(int frameCounter){
    const int step = 5;
    for(int x = frameCounter % step; x < SCREEN_WIDTH; x += step){
        for(int y = frameCounter % 2; y < SCREEN_HEIGHT; y += 2){
            Vec3 rayDir = GetRayDir(x,y,cam.dir);
            Vec3 colour = CastRay(cam.pos, rayDir, 0);
            screenBuffer[x][y] = colour;
        }
    }
    if(frameCounter % step == 0){
        //cam.pos += Vec3(0,0.05,0.1);
    }
}
void InitScreenBuffer(){
    screenBuffer.resize(SCREEN_WIDTH);
    looph(i, SCREEN_WIDTH){
        screenBuffer[i].resize(SCREEN_HEIGHT);
    }
}
void ClearScreenBuffer(){
    looph(i,screenBuffer.size()){
        looph(j,screenBuffer[i].size()){
            screenBuffer[i][j] = Vec3(0); 
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
            looph(j,3){nf.vertexList[j] += objList[i].pos;}
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
int main(){
    using namespace Graphics;
    InitScreenBuffer();

    #if false
            //Position used for testing test scene
        cam.pos = Vec3(-4,2,5);
        cam.dir = Vec3(0,deg2rad(100.f),0);
    #else
        //cam.pos = Vec3(0.38f, 1.f, 9.17f);
        //cam.dir = Vec3(0,3.63f, 0);
        cam.pos = Vec3(-6.51, 2, 8.59);
        cam.dir = Vec3(0, 2.3, 0);
    #endif

    string filePath = "Models/uploads_files_3581871_LION_STATUE_obj/me.obj";//"Models/uploads_files_3825299_Low+poly+bedroom_Obj/triModel.obj";
    objectList = ReadMeshFile(filePath, &worldMatList);
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
    #endif
    #if true
    Window window(SCREEN_WIDTH,SCREEN_HEIGHT,"Raytracer");
    window.Init();
    worldChunk.PrintInfo();

    window.StartLoop([](Graphics::Window* window){
        cout << "camPos=" << cam.pos << " camDir=" << cam.dir << "\n";
        #if true
            if(window->frameCounter <= 10)
                ExecuteRayTracer(window->frameCounter);
            else
                for(;;){}
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
