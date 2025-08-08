#pragma once
#include "Vec.hpp"
#include "Util.hpp"
struct Matrix{
    Vec3 lst[3];

    Matrix(){}
    Matrix(Vec3 lst[3]){
        memcpy(this->lst, lst, sizeof(Vec3)*3);
    }
    Matrix(Vec3 a, Vec3 b, Vec3 c){
        lst[0] = a;
        lst[1] = b;
        lst[2] = c;
    }
    Vec3& operator[](int index){
        #if SAFE == true
            if(index >= 3 || index < 0){
                cout << "Invalid index\n";
            }
        #endif
        return lst[index];
    }
    Matrix operator* (Matrix m){
        Matrix out;
        looph(x,3){
            looph(y,3){
                out[y][x] = dot(m[y], lst[y]);
            }
        }
        return out;
    }
    Vec3 operator* (Vec3 m){
        Vec3 out;
        looph(y,3){
            out[y] = m[0] * lst[y][0] + m[1] * lst[y][1] + m[2] * lst[y][2];
        }
        return out;
    }
};
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
        Vec3(cos(ang), -sin(ang), 0.f),
        Vec3(sin(ang), cos(ang), 0.f),
        Vec3(0.f, 0.f, 1.f)
    };
    return Matrix(lst);
}
