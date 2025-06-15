#ifdef SAFE_THING
#define SAFE true
//#define TRACY_ENABLE
//#define SAFE false
//#include "Tracy/tracy/Tracy.hpp"
#else
#define SAFE false
#endif
//#include <stdlib.h>

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
FixedArray<Mat> worldMatList;
#include "objLoader.hpp"


vector<vector<Vec3>> screenBuffer;
FixedArray<Face> worldFaceList;


struct SpaceChunk{
    //static constexpr real MIN_SIZE = 1.f;
    //FixedArray<SpaceChunk> lst;
    SpaceChunk* lst = nullptr;
    int spaceChunkNumber = 0;
    Face** faceList = nullptr;
    int faceNumber = 0;
    //FixedArray<Face*> faceList;
    Vec3 pos;
    real size;
    bool isLeaf = false;


    bool hit(Vec3 rayPos, Vec3 rayDir){
        #if true
        real v1 = (pos[0] - rayPos[0] - size/2.f) / rayDir[0];
        real v2 = (pos[0] - rayPos[0] + size/2.f) / rayDir[0];
        real minValue = std::min(v1,v2);
        real maxValue = std::max(v1,v2);
        loop(i,1,3){
            v1 = (pos[i] - rayPos[i] - size/2.f) / rayDir[i];
            v2 = (pos[i] - rayPos[i] + size/2.f) / rayDir[i];
            if(std::min(v1,v2) >= maxValue || std::max(v1,v2) <= minValue){
                return false;
            }
            maxValue = std::min(std::max(v1,v2), maxValue);
            minValue = std::max(std::min(v1,v2), minValue);
        }
        return maxValue > 0.f;
        #else
        
        Vec3 minVals;
        Vec3 maxVals;

        Vec3 minNumerator = pos - rayPos - size/2.f;
        Vec3 maxNumerator = pos - rayPos + size/2.f; 

        looph(i,3){
            if(rayDir[i] < 0.f){
                real temp = maxNumerator[i];
                maxNumerator[i] = minNumerator[i];
                minNumerator[i] = temp;
            }
        }

        minVals = minNumerator / rayDir;
        maxVals = maxNumerator / rayDir;

        real smallestMax = maxVals.min();
        if(minVals[0] > smallestMax||minVals[1] > smallestMax||minVals[2]>smallestMax){
            return false;
        }
        return (smallestMax>0.f);
        
       #endif

        /*
        //Algorithum for hitting self
        Vec3 minVals;
        Vec3 maxVals;
        //Optimise for SIMD
        looph(i,3){
            if(rayDir[i] >= 0){
                minVals[i] = (pos[i] - rayPos[i] - size/2.f) / rayDir[i];
                maxVals[i] = (pos[i] - rayPos[i] + size/2.f) / rayDir[i];
            }
            else{
                maxVals[i] = (pos[i] - rayPos[i] + size/2.f) / rayDir[i];
                minVals[i] = (pos[i] - rayPos[i] - size/2.f) / rayDir[i];
            }
        }
        real maxMinVal = minVals.max();
        real minMaxVal = maxVals.min();
        
        //return (maxMinVal > 0) && (maxMinVal > minMaxVal);
        return (minMaxVal > 0) && (maxMinVal > minMaxVal);   
        //return (minMaxVal > 0) && (maxMinVal < minMaxVal);    
        */
    }
    vector<Face*> IntersectingFaces(vector<Face*>* perantHitFaceList){
        //vector<Face*> l = *perantHitFaceList;
        vector<Face*> hitFaceList;
        looph(faceCounter,perantHitFaceList->size()){
            //#define face (*(*perantHitFaceList)[faceCounter])
            //Face face = *((*perantHitFaceList)[faceCounter]);
            Face* facePtr = (*perantHitFaceList)[faceCounter];

            real minx = facePtr->vertexList[0].x;
            real miny = facePtr->vertexList[0].y;
            real minz = facePtr->vertexList[0].z;

            real maxx = minx;
            real maxy = miny;
            real maxz = minz;

            loop(i,1,3){
                Vec3 vert = facePtr->vertexList[i];
                maxx = std::max(maxx, vert.x);
                maxy = std::max(maxy, vert.y);
                maxz = std::max(maxz, vert.z);

                minx = std::min(minx, vert.x);
                miny = std::min(miny, vert.y);
                minz = std::min(minz, vert.z);
            }
        
            if(minx <= pos.x + size/2.f && maxx >= pos.x-size/2.f){
                if(miny <= pos.y+size/2.f && maxy >= pos.y - size/2.f){
                    if(minz <= pos.z + size/2.f && maxz >= pos.z-size/2.f){
                        hitFaceList.push_back(facePtr);
                    }
                }
            }
            //#undef face
        }
        //If smallest space split then save faces to self
        if(hitFaceList.size() <= SPACE_CHUNK_MAX_FACES && hitFaceList.size() > 0){
            //faceList.AllocArray(hitFaceList.size());
            faceList = new Face*[hitFaceList.size()];
            faceNumber = hitFaceList.size();
            looph(i,hitFaceList.size()){
                faceList[i] = hitFaceList[i];
            }
            isLeaf = true;
            //Sets size so perant knows child is not empty
            //return FixedArray<Face*>(hitFaceList.size());
        }
        //
        //FixedArray<Face*> ret(hitFaceList.size());
        //memcpy(ret.ptr(), &hitFaceList[0], ret.size() * sizeof(Face*));
        //return ret;
        return hitFaceList;
    }
    void Init(vector<Face*>* faceList){
        cout << "Size = " << size << "number of faces of perant = " << faceList->size() << "\n";
        //1. Create array of all child space chunks
        SpaceChunk  childList[sq(SPACE_CHUNCK_SPLIT)*SPACE_CHUNCK_SPLIT];
        // 2. Initilize their values
        looph(x,SPACE_CHUNCK_SPLIT){
            looph(y,SPACE_CHUNCK_SPLIT){
                looph(z,SPACE_CHUNCK_SPLIT){
                    SpaceChunk& child = childList[x*sq(SPACE_CHUNCK_SPLIT)+y*SPACE_CHUNCK_SPLIT+z];
                    /*
                    child.pos = pos + Vec3(x*(size/(real)SPACE_CHUNCK_SPLIT),0,0)
                                    - Vec3(0,y*(size/(real)SPACE_CHUNCK_SPLIT),0)
                                    + Vec3(0,0,z*(size/(real)SPACE_CHUNCK_SPLIT));
                    */
                    real stepSize = size / (real)SPACE_CHUNCK_SPLIT;
                    child.pos = pos - Vec3(stepSize/2.f) + Vec3(
                        stepSize * x,
                        stepSize * y,
                        stepSize * z
                    );
                    child.size = stepSize;
                    //Initilize ptr
                    child.faceList = nullptr;
                    child.lst = nullptr;
                }   
            }
        }
        // 3. check face hit children
        int numberOfChildren = 0;
        int nonEmptyChildIndexs[numberof(childList)];
        int numberOfHitFaces[numberof(childList)];
        memset(numberOfHitFaces, 0, sizeof(numberOfHitFaces));
        memset(nonEmptyChildIndexs, 0xff, sizeof(nonEmptyChildIndexs));
        vector<vector<Face*>> listOfHitFaceLists;
        listOfHitFaceLists.reserve(numberof(childList));
        int numberOfFacesInChildren = 0;
        looph(i,numberof(childList)){
            vector<Face*> childHitFaceList = childList[i].IntersectingFaces(faceList);
            //if(childHitFaceList.size() == faceList->size())
            //{
            //    noMoreChildren = true;
            //    break;
            //}
            numberOfFacesInChildren += childHitFaceList.size();
            if(childHitFaceList.size() > 0){
                numberOfHitFaces[i] = childHitFaceList.size();
                //if(childList[i].isLeaf == false){
                //    childList[i].Init(&childHitFaceList);
                //}
                nonEmptyChildIndexs[i] = true;
                numberOfChildren++;
            }      
            listOfHitFaceLists.push_back(childHitFaceList);     
        }
        //bool noMoreChildren = false;
        /*
        real avgRemoveFacePercent = 0.f;
        looph(i,numberof(childList)){
            if(listOfHitFaceLists[i].size() > 0){
                avgRemoveFacePercent += (real)std::labs(listOfHitFaceLists[i].size() - faceList->size()) / (real)faceList->size();
            }            
        }
        avgRemoveFacePercent /= numberOfChildren;
        */
        real limit = (real)faceList->size() * SPACE_CHUNK_DUPLICATE_FACE;
        if(numberOfFacesInChildren >= limit){
            //Set the current node to be a leaf node
            this->faceList = new Face*[faceList->size()];
            faceNumber = faceList->size();
            looph(i,faceNumber){
                this->faceList[i] = (*faceList)[i];
            }
            isLeaf = true;
            //Children might have allocated memory so clean up their memory
            looph(i,numberof(childList)){
                childList[i].FreeMemory();
            }            
        }
        else{
            //Initlize children that need to be initilized
            looph(i,numberof(childList)){
                if(childList[i].isLeaf == false && listOfHitFaceLists[i].size() > 0){
                    childList[i].Init(&(listOfHitFaceLists[i]));
                }
            }
            //Save child list to fixed array
            //lst.AllocArray(numberOfChildren);
            lst = new SpaceChunk[numberOfChildren];
            spaceChunkNumber = numberOfChildren;
            int j = 0;
            looph(i,numberof(childList)){
                if(nonEmptyChildIndexs[i] != -1){
                    lst[j++] = childList[i];
                }            
            }
        }
    }
    void RemoveNodesWithSingleChild(){
        looph(i,spaceChunkNumber){
            if(lst[i].spaceChunkNumber == 1){
                cout << "Removed node with single child\n";
                //Create copy of child all points should remain valid
                SpaceChunk childOfChild = lst[i].lst[0];
                //Destroy child
                lst[i].FreeMemory();
                //Replace with child
                lst[i] = childOfChild;
            }
        }
        looph(i,spaceChunkNumber){
            lst[i].RemoveNodesWithSingleChild();
        }
    }
    void Create4VertFaces(vector<Face*>* faceRemovalList){
        //cout << "Running 4 vec\n";
        //vector<Face*> faceRemovalList;
        //1.Find matching vertecies
        looph(faceCounter1, faceNumber){
            Face* facePtr1 = faceList[faceCounter1];
            if(in(*faceRemovalList, facePtr1)){continue;}
            looph(faceCounter2, faceNumber){
                Face* facePtr2 = faceList[faceCounter2];
                if(facePtr1 == facePtr2){continue;}
                if(in(*faceRemovalList, facePtr2)){continue;}
                //Check if two verticies the same
                int sameCounter = 0;
                //Record vertex that is not the same
                int notSameIndexForFace1 = -1;                
                looph(vertCounter1, 3){
                    Vec3 vert = facePtr1->vertexList[vertCounter1];
                    int equalToAny = vertCounter1;
                    looph(vertCounter2, 3){
                        if(vert == facePtr2->vertexList[vertCounter2]){
                            sameCounter++;
                            equalToAny = -1;
                            break;
                        }
                    }
                    if(equalToAny != -1){
                        notSameIndexForFace1 = equalToAny;
                    }
                }
                //if two vertex arent the same then face pair dont work
                if(sameCounter != 2){
                    continue;
                }
                //Get index of other vert that is not the same
                int notSameIndexForFace2 = -1;
                looph(vertCounter2, 3){
                    notSameIndexForFace2 = vertCounter2;
                    looph(vertCounter1, 3){
                        if(facePtr1->vertexList[vertCounter1] == facePtr2->vertexList[vertCounter2]){
                            notSameIndexForFace2 = -1;
                        }
                    }
                    if(notSameIndexForFace2 != -1){
                        break;
                    }
                }
                #if false
                looph(i,3){
                    cout << "Vert " << facePtr1->vertexList[i] << "\n";
                }
                looph(i,3){
                    cout << "Vert " << facePtr2->vertexList[i] << "\n";
                }
                cout << "Non hit vert 1 " << facePtr1->vertexList[notSameIndexForFace1] << "\n";
                cout << "Non hit vert 2 " << facePtr2->vertexList[notSameIndexForFace2] << "\n";
                #endif

                //Compare non-same vertex
                //P is not same for face 1, F is not same for face 2
                //P + I + J = F
                Vec3 P = facePtr1->vertexList[notSameIndexForFace1];
                Vec3 I = facePtr1->vertexList[(notSameIndexForFace1 + 1) % 3] - P;
                Vec3 J = facePtr1->vertexList[(notSameIndexForFace1 + 2) % 3] - P;
                Vec3 F = facePtr2->vertexList[notSameIndexForFace2];
                if(P + I + J == F){
                    //cout << "4 vertexing face! face index 1 and 2 " << faceCounter1 << ", " <<faceCounter2 <<"\n";
                    //Set face 1 to be 4 vertex
                    facePtr1->isRectangle = true;
                    //IMPORTANT
                    //not same vertex NEEDS to be the first vertex so then I and J vectors are correct
                    if(notSameIndexForFace1 != 0){
                        Vec3 temp = facePtr1->vertexList[0];
                        facePtr1->vertexList[0] = facePtr1->vertexList[notSameIndexForFace1];
                        facePtr1->vertexList[notSameIndexForFace1] = temp;
                        notSameIndexForFace1 = 0;                        
                    }
                    

                    faceRemovalList->push_back(facePtr2);
                    /*
                    //Re alloc face list and remove pointer to dud face.
                    Face** newFaceList = new Face*[faceNumber-1];
                    looph(i,faceNumber-1){
                        if(faceList[i] == facePtr2){continue;}
                        newFaceList[i] = faceList[i];
                    }
                    faceNumber--;
                    //free(faceList);
                    delete[] faceList;
                    faceList = newFaceList;
                    //Deleteing current face so allready pointing to next face
                    faceCounter2--;
                    */
                }
            }
        }
        //Remove dud faces
        /*
        if(faceRemovalList.size() > 0){
            int newFaceSize = faceNumber - faceRemovalList.size();
            Face** newFaceList = new Face*[newFaceSize];
            int newFaceCounter = 0;
            looph(i,faceNumber){
                if(in(faceRemovalList, faceList[i])){continue;}
                newFaceList[newFaceCounter] = faceList[i];
                newFaceCounter++;
            }       
            delete[] faceList;
            faceList = newFaceList;
            faceNumber = newFaceSize; 
        }  
            */     
        //4.Call function on children
        looph(i,spaceChunkNumber){
            lst[i].Create4VertFaces(faceRemovalList);
        }
    }
    void RemoveDudFaces(vector<Face*>* ptr){
        vector<int> idxToRemove;
        looph(i, faceNumber){
            looph(j, ptr->size()){
                if((*ptr)[j] == faceList[i]){
                    idxToRemove.push_back(i);
                    break;
                }
            }            
        }
        if(idxToRemove.size() > 0){
            int newSize = faceNumber - idxToRemove.size();
            Face** newLst = new Face*[newSize];
            int j = 0;
            looph(i,faceNumber){
                if(in(idxToRemove,i)){continue;}
                newLst[j] = faceList[i];
                j++;
            }
            delete[] faceList;
            faceList = newLst;
            faceNumber = newSize;
        }
        looph(i,spaceChunkNumber){
            lst[i].RemoveDudFaces(ptr);
        }
    }
    void PrintInfo(int tabNumber=0){
        cout << string(tabNumber, '-');
        cout << "SC childNumber=" << spaceChunkNumber << " faceCount=" << faceNumber << " size="<< size << " pos=" << pos << " \n";
        looph(i,spaceChunkNumber){
            lst[i].PrintInfo(tabNumber+1);
        }
    }
    void SetSizeAndPos(){
        Vec3 avgPos = Vec3(0,0,0);
        real maxSize = 0.f;
        uint vertCount = 0;
        looph(i,worldFaceList.size()){
            Face& f = worldFaceList[i];
            looph(j,3){
                avgPos += f.vertexList[j];                
                vertCount++;
            }
        }
        avgPos /= vertCount;
        looph(i,worldFaceList.size()){
            Face& f = worldFaceList[i];
            looph(j,3){
               looph(k,3){
                    maxSize = std::max(maxSize,std::abs(f.vertexList[j][k] - avgPos[k]));
                } 
            }
        }        
        size = maxSize*2.f;
        pos = avgPos;
    }
    void FreeMemory(){
        if(lst != nullptr){
            delete[] lst;
        }
        if(faceList != nullptr){
            delete[] faceList;
        }
    }
};
struct Cam{
    Vec3 dir = Vec3(0);
    Vec3 pos = Vec3(0);
};

Cam cam;
SpaceChunk worldChunk;
vector<Object> objectList;


Matrix XRotationMatrix(real ang){
    using std::cos; using std::sin;
    Vec3 lst[3] = {
        Vec3(1,0,0),
        Vec3(0,cos(ang), -sin(ang)),
        Vec3(0, sin(ang), cos(ang))
    };
    return Matrix(lst);
}
Matrix YRotationMatrix(real ang){
    using std::cos; using std::sin;
    Vec3 lst[3] = {
        Vec3(cos(ang),0,sin(ang)),
        Vec3(0, 1, 0),
        Vec3(-sin(ang), 0, cos(ang))
    };
    return Matrix(lst);
}
Matrix ZRotationMatrix(real ang){
    using std::cos; using std::sin;
    Vec3 lst[3] = {
        Vec3(cos(ang) ,-sin(ang), 0),
        Vec3(sin(ang), cos(ang), 0),
        Vec3(0, 0, 1)
    };
    return Matrix(lst);
}
real ApproxSin(real angle){
    real sign = (angle < 0.f) ? -1.f : 1.f;
    real normAngle = std::abs(angle) / (2.f * PI);
    normAngle -= std::floor(normAngle);
    
    real o = (normAngle < 0.5f) ? normAngle : -(normAngle-1.f);
    return 16.f * (normAngle-0.5f) * o * sign;
    //return 20.6f * normAngle * (normAngle - 0.5f) * (normAngle - 1.f) * sign;
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
    Vec3 ray_camera = Vec3(-x_camera, y_camera, 1.0f);//.normalize();

    // Rotate to world space using camRot (assumes row-major matrix mul order)
    Vec3 ray_world = XRotationMatrix(camRot.x) * (YRotationMatrix(camRot.y) * (ZRotationMatrix(camRot.z) * ray_camera));

    return ray_world;//.normalize();
}
//Returns array with closes space chunk at the end
void GetNextHitSpaceChunk(Vec3 rayPos, Vec3 rayDir, vector<SpaceChunk*>* spaceChunkList,bool dontExpandNextSpaceChunk, int* ptr){
    //1. Remove last node as it didn't hit anything in it
    spaceChunkList->pop_back();
    //2. Check hit last node
    while(spaceChunkList->size() != 0){
        SpaceChunk* last = (*spaceChunkList)[spaceChunkList->size()-1];
        if(last->hit(rayPos, rayDir)){   
            (*ptr)++;         
            if(last->isLeaf){
                //x. Return when base level node is hit
                break;
            }
            else{
                //When hit node but doesnt know if minDistance is actually min distance only
                //need to look at nodes on the same level, perchance im not too sure
                //if(dontExpandNextSpaceChunk){
                //    spaceChunkList->resize(0);
                //    return;
                //}
                    
                //3. Expand hit node
                spaceChunkList->pop_back();

                using std::pair;
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
                    [](const auto& a, const auto& b) ->bool{
                        return a.second > b.second;
                    }
                );
                //add all entries onto the end of the vector 
                //(*spaceChunkList).insert(spaceChunkList->end(), buffer, buffer+numberAdded);              
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
    return Vec3(0.9,0.9,1.f) * std::min(std::max(0.2f,a*2.f), 1.f);
}
Vec3 GetReflectedRayDir(Vec3 incomingRayDir, Vec3 faceNormal, Face* facePtr, int rayNumber, int numberOfSamples){
    //using std::cos; using std::sin;
    const real goldenRatio = (1.f + sqrt(5.f)) / 2.f;
    real x = (real)rayNumber / goldenRatio;
    x = x - std::floor(x);
    real y = (real)rayNumber / (real)(numberOfSamples-1);///2.f means only generate for hemisphere
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
    );//.normalize();
}
Vec3 CastRay(Vec3 rayPos, Vec3 rayDir, int bounceNumber, vector<SpaceChunk*>& spaceChunkList,
    Face* cantHitFace=nullptr)
{
    //Get list of SpaceChunks hit in order of distance
    //May not even need to heap alloc
    //vector<SpaceChunk*> spaceChunkList;
    //spaceChunkList.reserve(pow3(SPACE_CHUNCK_SPLIT) * 3);
    spaceChunkList = {&worldChunk, nullptr};
    bool neverHitFace = true;
    //Vec3 colour = GetSkyColour(rayDir);
    Vec3 colour = Vec3(0);
    int spaceChunkHitCounter = 0;
    real minDistance = FLOAT_MAX_VALUE; 
    bool dontExpandNextSpaceChunk = false;
    while(true){
        GetNextHitSpaceChunk(rayPos, rayDir, &spaceChunkList,dontExpandNextSpaceChunk, &spaceChunkHitCounter);
        //colour += Vec3(0.05,0,0);
        if(spaceChunkList.size() == 0){break;}
        //spaceChunkHitCounter++;
        SpaceChunk* chunkToCheck = spaceChunkList[spaceChunkList.size()-1];               
        Face* hitFacePtr = nullptr;

        //colour += Vec3(0,0.1,0);
        //neverHitFace = false;
        //break;
        
        looph(faceCounter, chunkToCheck->faceNumber){
            Face* currentFacePtr = chunkToCheck->faceList[faceCounter];
            if(currentFacePtr == cantHitFace){continue;}
            //Face collision function called here
            real t = RayFaceCollision(rayPos, rayDir, currentFacePtr);
            if(t > EPSILON && t < minDistance){
                minDistance = t;
                hitFacePtr = currentFacePtr;
            }
        }
        if(hitFacePtr != nullptr){
            //return colour;
            #if true
            colour = hitFacePtr->mat->colour * hitFacePtr->mat->em;
            if(bounceNumber < MAX_BOUNCES && hitFacePtr->mat->em < 1.f){
                Vec3 avgOfColours = Vec3(0.f,0.f,0.f);
                const int SAMPLE_COUNT = SAMPLES_FOR_BOUNCE_NUMBER[bounceNumber];
                Vec3 faceNormal = hitFacePtr->normal;
                if(dot(faceNormal, rayDir) > 0.f){
                    faceNormal *= -1.f;
                }
                looph(rayCounter, SAMPLE_COUNT){
                    Vec3 newDir = GetReflectedRayDir(rayDir, faceNormal, hitFacePtr,  rayCounter, SAMPLE_COUNT);
                    avgOfColours += CastRay(rayDir*minDistance + rayPos, newDir, bounceNumber + 1, spaceChunkList,
                         hitFacePtr);
                    //cout << newDir.x << " "<< newDir.y << " "<< newDir.z << "\n";
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
            //colour = Vec3(1,0.1,0.1) * 1.f/minDistance;
            //colour = Vec3(1,0.1,0.1) * 1.f/(real)spaceChunkHitCounter;
            #endif
            neverHitFace = false;
            //
            //if(sq(minDistance) > ())
            //dontExpandNextSpaceChunk = true;
            break;
        }
    }
    //colour = Vec3(spaceChunkHitCounter==2,0.1,0.1);
    if(neverHitFace){
        colour = GetSkyColour(rayDir);
        //colour += GetSkyColour(rayDir);
    }
    return colour;
}

void ExecuteRayTracer(int frameCounter){
    const int step = 5;
    vector<SpaceChunk*> spaceChunkList;
    spaceChunkList.reserve(pow3(SPACE_CHUNCK_SPLIT) * 7);
    for(int x = frameCounter % step; x < SCREEN_WIDTH; x += step){
        for(int y = frameCounter % 2; y < SCREEN_HEIGHT; y += 2){
            Vec3 rayDir = GetRayDir(x,y,cam.dir);
            Vec3 colour = CastRay(cam.pos, rayDir, 0, spaceChunkList);
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
        //window->Pump();
    }
}
void GenerateWorldFaceList(vector<Object>& objList){
    /*
    //1.Get list of all faces and make their positions relitive to world pos
    vector<Face> faceList;
    looph(i,objList.size()){
        looph(f,objList[i].faceList.size()){
            Face face = objList[i].faceList[f];
            looph(j,3){
                face.vertexList[j] += objList[i].pos;
            }
            faceList.push_back(face);
        }
    }
    //2.Look for all faces with same two vertex then check if rectangle / rhombus
    looph(faceCounter, faceList.size()){

    }
    //3.Allocate fixed size array and set normals
    */
    
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
/*
$process = Start-Process "your_application.exe" -PassThru
$process.Id

*/
int main(){
    using namespace Graphics;
    InitScreenBuffer();

   //cam.pos = Vec3(-4,2,12);
   //cam.dir = Vec3(0,deg2rad(150.f),0);

   cam.pos = Vec3(-4,2,5);
   cam.dir = Vec3(0,deg2rad(100.f),0);

   string filePath = "/home/william/Documents/Prog/GitHubRepos/SoftwareRayTracer/Models/uploads_files_3825299_Low+poly+bedroom_Obj/triModel.obj";
   objectList = ReadMeshFile(filePath);
   objectList[objectList.size()-1].pos = Vec3(-2,-2,5);

    GenerateWorldFaceList(objectList);
    //PrintFaceList(worldFaceList.lst, worldFaceList.size());
    vector<Face*> ptrList = GetWorldFacePtrList();
    //worldChunk.size = 2;
    //worldChunk.pos = Vec3(-1,1,2);
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
 

        // Draw a red pixel at (100, 100)
        //window->DrawPixel(100, 100, Vec3(1.0f, 0.0f, 0.0f));
        cout << "New Frame\n";
        #if false
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
        //cout << cam.dir << "\n";
        ExecuteRayTracer(window->frameCounter);
        #endif
        DrawScreenBuffer(window);
        //cout << window->IsKeyPressed(XK_w) << "\n";

        


        //window->DrawPixel(10,10, 
        //            Vec3(0.5,0.5,0));
        /*

        int counter = 0;
        looph(r,255){
            looph(g,255){
                looph(b,255){
                    window->DrawPixel(counter%SCREEN_WIDTH, (counter / SCREEN_WIDTH) % SCREEN_HEIGHT, 
                    Vec3((real)r / 255.f,(real)g / 255.f,(real)b / 255.f));
                    counter++;
                }
            }
        }
        */
    });
    #else
    looph(frameCounter, 1){
        //cout << "Frame " << frameCounter << "\n";
        looph(i,5*2){
            ExecuteRayTracer(i);
        }
    }

    
    #endif
}