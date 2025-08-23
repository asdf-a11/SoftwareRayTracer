//Memory must be freed manually, this is to allow for copying without reallocation
struct SpaceChunk{
    SpaceChunk* lst = nullptr;
    int spaceChunkNumber = 0;//could be unsigned char to improve cache locality but check would be needed to avoid overflow
    Face** faceList = nullptr;
    int faceNumber = 0;
    Vec3 pos;
    real size;
    bool isLeaf = false;

    bool hit(Vec3 rayPos, Vec3 rayDir){
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
    }
    vector<Face*> IntersectingFaces(vector<Face*>* perantHitFaceList){
        vector<Face*> hitFaceList;
        looph(faceCounter,perantHitFaceList->size()){
            Face* facePtr = (*perantHitFaceList)[faceCounter];

            real minx = facePtr->v0.x;
            real miny = facePtr->v0.y;
            real minz = facePtr->v0.z;

            real maxx = minx;
            real maxy = miny;
            real maxz = minz;

            Vec3 vertLst[2] = {facePtr->v0v1+facePtr->v0,facePtr->v0v2+facePtr->v0};
            loop(i,1,3){
                Vec3 vert = vertLst[i-1];
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
        }
        //If smallest space split then save faces to self
        if(hitFaceList.size() <= SPACE_CHUNK_MAX_FACES && hitFaceList.size() > 0){
            faceList = new Face*[hitFaceList.size()];
            faceNumber = hitFaceList.size();
            looph(i,hitFaceList.size()){
                faceList[i] = hitFaceList[i];
            }
            isLeaf = true;
        }
        return hitFaceList;
    }
    void Init(vector<Face*>* faceList){
        //1. Create array of all child space chunks
        SpaceChunk  childList[sq(SPACE_CHUNCK_SPLIT)*SPACE_CHUNCK_SPLIT];
        // 2. Initilize their values
        looph(x,SPACE_CHUNCK_SPLIT){
            looph(y,SPACE_CHUNCK_SPLIT){
                looph(z,SPACE_CHUNCK_SPLIT){
                    SpaceChunk& child = childList[x*sq(SPACE_CHUNCK_SPLIT)+y*SPACE_CHUNCK_SPLIT+z];
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
            numberOfFacesInChildren += childHitFaceList.size();
            if(childHitFaceList.size() > 0){
                numberOfHitFaces[i] = childHitFaceList.size();
                nonEmptyChildIndexs[i] = true;
                numberOfChildren++;
            }      
            listOfHitFaceLists.push_back(childHitFaceList);     
        }
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
                Vec3 vertexList[3] = {
                    facePtr1->v0,
                    facePtr1->v0v1+facePtr1->v0,
                    facePtr1->v0v2+facePtr1->v0
                };             
                looph(vertCounter1, 3){
                    Vec3 vert = vertexList[vertCounter1];
                    int equalToAny = vertCounter1;
                    looph(vertCounter2, 3){
                        if(vert == vertexList[vertCounter2]){
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
                        if(vertexList[vertCounter1] == vertexList[vertCounter2]){
                            notSameIndexForFace2 = -1;
                        }
                    }
                    if(notSameIndexForFace2 != -1){
                        break;
                    }
                }

                //Compare non-same vertex
                //P is not same for face 1, F is not same for face 2
                //P + I + J = F
                Vec3 P = vertexList[notSameIndexForFace1];
                Vec3 I = vertexList[(notSameIndexForFace1 + 1) % 3] - P;
                Vec3 J = vertexList[(notSameIndexForFace1 + 2) % 3] - P;
                Vec3 F = vertexList[notSameIndexForFace2];
                if(P + I + J == F){
                    //Set face 1 to be 4 vertex
                    facePtr1->isRectangle = true;
                    //IMPORTANT
                    //not same vertex NEEDS to be the first vertex so then I and J vectors are correct
                    if(notSameIndexForFace1 != 0){
                        Vec3 temp = vertexList[0];
                        vertexList[0] = vertexList[notSameIndexForFace1];
                        vertexList[notSameIndexForFace1] = temp;
                        notSameIndexForFace1 = 0;                        
                    }
                    faceRemovalList->push_back(facePtr2);
                }
            }
        }   
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
        if(spaceChunkNumber==0){
            cout << string(tabNumber, '-');
            cout << "SC childNumber=" << spaceChunkNumber << " faceCount=" << faceNumber << " size="<< size << " pos=" << pos << " \n";
        }
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
            Vec3 vertexList[3] = {
                f.v0,
                f.v0v1+f.v0,
                f.v0v2+f.v0
            }; 
            looph(j,3){
                avgPos += vertexList[j];                
                vertCount++;
            }
        }
        avgPos /= vertCount;
        looph(i,worldFaceList.size()){
            Face& f = worldFaceList[i];
            Vec3 vertexList[3] = {
                f.v0,
                f.v0v1+f.v0,
                f.v0v2+f.v0
            }; 
            looph(j,3){
               looph(k,3){
                    maxSize = std::max(maxSize,std::abs(vertexList[j][k] - avgPos[k]));
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