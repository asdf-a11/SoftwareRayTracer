#pragma once
#include "Vec.hpp"
#include "Util.hpp"
struct Matrix2x2{
    Vec2 rows[2];
    
    Matrix2x2(){}
    Matrix2x2(Vec2 lst[2], bool isRows=false){
        if(isRows){
            looph(x,2){
                looph(y,2){
                    rows[y][x] = lst[x][y];
                }
            }
        }else{
            looph(i,2){
                rows[i] = lst[i];
            }
        }
    }
    real det(){
        return rows[0][0] * rows[1][1] - rows[0][1] * rows[1][0];
    }
};  
struct Matrix3x3{
    Vec3 rows[3];

    Matrix3x3(){}
    Matrix3x3(Vec3 lst[3], bool isRows=false){
        if(isRows){
            looph(x,3){
                looph(y,3){
                    rows[y][x] = lst[x][y];
                }
            }
        }else{
            looph(i,3){
                rows[i] = lst[i];
            }
        }
    }
    Vec3& operator[](int index){
        #if SAFE == true
            if(index >= 3 || index < 0){
                cout << "Invalid index\n";
            }
        #endif
        return rows[index];
    }
    Matrix3x3 operator* (Matrix3x3 m){
        Matrix3x3 out;
        looph(x,3){
            looph(y,3){
                out[y][x] = dot(m[y], rows[y]);
            }
        }
        return out;
    }
    Vec3 operator* (Vec3 m){
        Vec3 out;
        looph(y,3){
            out[y] = m[0] * rows[y][0] + m[1] * rows[y][1] + m[2] * rows[y][2];
        }
        return out;
    }

    Matrix3x3 operator/(real v){
        Matrix3x3 out;
        looph(y,3){
            looph(x,3){
                out[y][x] = rows[y][x] / v;
            }
        }
        return out;
    }

    Matrix3x3 inverse(){
        Matrix3x3 out;
        Matrix3x3 detMatrix;
        looph(x,3){
            looph(y,3){
                Matrix2x2 minor;
                int minorYCounter = 0;
                int minorXCounter = 0;
                looph(x2,3){
                    looph(y2,3){
                        if(x == x2 || y == y2) continue;
                        minor.rows[minorYCounter][minorXCounter++] = rows[y2][x2];
                        if(minorXCounter == 2){
                            minorXCounter = 0;
                            minorYCounter++;
                        }
                    }
                }
                real det = minor.det();
                detMatrix[y][x] = det;
            }
        }
        //Apply cofactor matrix
        real sign = 1.f;
        looph(y,3){
            looph(x,3){
                detMatrix[y][x] *= sign;
                sign *= -1.f;
            }
        }
        //calculate determinant of orignal 3x3 matrix
        Vec3 v = (detMatrix.rows[0] * rows[0]);
        real detOfOrignalMatrix = v.sum();
        //Transpose
        detMatrix = detMatrix.transpose();
        return detMatrix / detOfOrignalMatrix;
    }

    Matrix3x3 transpose(){
        Matrix3x3 out;
        looph(y,3){
            looph(x,3){
                out.rows[x][y] = rows[y][x];
            }
        }
        return out;
    }
};

Matrix3x3 XRotationMatrix(real ang){
    using std::cos; using std::sin;
    Vec3 lst[3] = {
        Vec3(1,0,0),
        Vec3(0,cos(ang), -sin(ang)),
        Vec3(0, sin(ang), cos(ang))
    };
    return Matrix3x3(lst);
}
Matrix3x3 YRotationMatrix(real ang){
    using std::cos; using std::sin;
    Vec3 lst[3] = {
        Vec3(cos(ang),0,sin(ang)),
        Vec3(0, 1, 0),
        Vec3(-sin(ang), 0, cos(ang))
    };
    return Matrix3x3(lst);
}
Matrix3x3 ZRotationMatrix(real ang){
    using std::cos; using std::sin;
    Vec3 lst[3] = {
        Vec3(cos(ang), -sin(ang), 0.f),
        Vec3(sin(ang), cos(ang), 0.f),
        Vec3(0.f, 0.f, 1.f)
    };
    return Matrix3x3(lst);
}
