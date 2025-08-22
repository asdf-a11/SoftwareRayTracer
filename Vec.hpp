#pragma once
#include <cmath>
#include "Util.hpp"
#include "Settings.hpp"
template<int size> struct Vec {
    union{
        struct {real x,y,z;};
        struct {real r,g,b;};
        real lst[size];
    };
    Vec(){}
    //Vec(real list[size]){
    //    looph(i,size){
    //        lst[i] = list[i];
    //    }
    //}
    Vec(real v){
        looph(i,size){
            lst[i] = v;
        }
    }
    Vec(real x, real y){
        lst[0] = x; lst[1] = y;
        if(size != 2){
            cerr << "Bad stuff\n";
            exit(EXIT_FAILURE);
        }
    }
    Vec(real x, real y, real z){
        lst[0] = x; lst[1] = y; lst[2] = z;
        if(size != 3){
            cerr << "Bad stuff\n";
            exit(EXIT_FAILURE);
        }
    }

    #define C(op) \
        Vec<size> operator op (Vec<size> v)  { \
            Vec<size> nv;\
            looph(i,size){\
                nv.lst[i] = lst[i] op v.lst[i];\
            }\
            return nv;\
        }
        C(+)
        //C(-)
        C(*)
        C(/)
    #undef C
    #define C(op) \
        Vec<size> operator op (real v){\
            Vec<size> nv;\
            looph(i,size){\
                nv.lst[i] = lst[i] op v;\
            }\
            return nv;\
        }
        C(+)
        C(-)
        C(*)
        C(/)
    #undef C
    
    void operator+=(Vec<size> a){
        *this = *this + a;
    }
    void operator*=(Vec<size> a){
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
    Vec<size>& operator=(real v){
        looph(i,size){
            lst[i] = v;
        }
        return *this;
    }

    Vec<size> pow(real exponent){
        Vec<size> nv;
        looph(i,size){
            nv.lst[i] = std::powf(lst[i],exponent);
        }
        return nv;
    }
    real sum(){
        real out = 0.f;
        looph(i,size){
            out += lst[i];
        }
        return out;
    }
    real lengthSquared(){
        return (this->pow(2.f)).sum();
    }
    real length() {
        return std::sqrtf((this->pow(2.f)).sum());
    }
    real max(){
        real out = lst[0];
        looph(i,size){
            out = std::max(out,lst[i]);
        }
        return out;
    }
    real min(){
        real out = lst[0];
        looph(i,size){
            out = std::min(out,lst[i]);
        }
        return out;
    }
    Vec<size> normalize(){
        return *this / length();
    }
    bool operator==(Vec<size> v){
        bool isEqual = true;
        looph(i,size){
            isEqual = isEqual && std::abs(lst[i]-v.lst[i]) < EPSILON;
        }
        return isEqual;
    }
    bool operator!=(Vec<size> v){
        return !this->operator==(v);
    }

};

#define Vec3 Vec<3>
#define Vec2 Vec<2>

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
Vec2 operator- (Vec2 a, Vec2 b){
    return Vec2(
        a.x - b.x,
        a.y - b.y
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

/*
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
*/