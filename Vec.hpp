#pragma once
#include <cmath>
#include "Util.hpp"
#include "Settings.hpp"
struct Vec3{
    union{
        struct {real x,y,z;};
        struct {real r,g,b;};
        real lst[3];
    };
    Vec3(){}
    Vec3(real x, real y, real z){
        lst[0] = x;
        lst[1] = y;
        lst[2] = z;
    }
    Vec3(real v){
        lst[0] = v; lst[1] = v; lst[2] = v;
    }

    #define C(op) \
        Vec3 operator op (Vec3 v)  { \
            return Vec3(\
                x op v.x, \
                y op v.y, \
                z op v.z \
            ); \
        }
        C(+)
        //C(-)
        C(*)
        C(/)
    #undef C
    #define C(op) \
        Vec3 operator op (real v){\
            return Vec3(\
                x op v, \
                y op v, \
                z op v \
            ); \
        }
        C(+)
        C(-)
        C(*)
        C(/)
    #undef C
    
    void operator+=(Vec3 a){
        *this = *this + a;
    }
    void operator*=(Vec3 a){
        *this = *this * a;
    }
    void operator/=(real a){
        *this = *this / a;
    }

    real& operator[](int index){
        #if SAFE == true
            if(index >= 3 || index < 0){
                cout << "Error\n";
            }
        #endif
        return lst[index];
    }
    real lengthSquared(){
        return sq(x) + sq(y) + sq(z);
    }
    real length() {
        return std::sqrt(lengthSquared());
    }
    real max(){
        return std::max(std::max(x,y),z);
    }
    real min(){
        return std::min(std::min(x,y),z);
    }
    Vec3 normalize(){
        return *this / length();
    }
    bool operator==(Vec3 v){
        return std::abs(x-v.x) < EPSILON &&
               std::abs(y-v.y) < EPSILON &&
               std::abs(z-v.z) < EPSILON;
    }
    bool operator!=(Vec3 v){
        return !this->operator==(v);
    }

};

real dot(Vec3 a, Vec3 b){
    return a.x*b.x + a.y*b.y + a.z*b.z;
}
Vec3 cross(Vec3 v_A, Vec3 v_B){
    Vec3 c_P;
    c_P[0] = v_A[1] * v_B[2] - v_A[2] * v_B[1];
    c_P[1] = -(v_A[0] * v_B[2] - v_A[2] * v_B[0]);
    c_P[2] = v_A[0] * v_B[1] - v_A[1] * v_B[0];
    return c_P;
}

Vec3 operator- (Vec3 a, Vec3 b){
    return Vec3(
        a.x - b.x,
        a.y - b.y,
        a.z - b.z
    );
}
//bool operator!= (Vec3 a, Vec3 b){
//    bool out = true;
//    looph(i,3){
//        out = out && (a[i] == b[i]);
//    }
//    return !out;
//}
std::ostream &operator<<(std::ostream &os, Vec3 const &m) { 
    return os << "Vec3(" << m.x << ", " << m.y << ", " << m.z << ")";
}

#define C(op) \
    bool operator op (Vec3 a, Vec3 b){\
        return (a.x op b.x) && (a.y op b.y) && (a.z op b.z);\
    }

C(<=);
C(>=);
C(<);
C(>);

#undef C