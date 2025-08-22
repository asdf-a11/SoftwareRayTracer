#pragma once
#include "Util.hpp"
#include "Vec.hpp"

struct Image {
    int  width  = 0;
    int  height = 0;
    byte* buffer = nullptr;

    Image() = default;

    // Allocate our own buffer (zero-initialized)
    explicit Image(int w, int h)
        : width(w), height(h),
          buffer((w > 0 && h > 0) ? new byte[w * h * 3]() : nullptr) {}

    // Copy from external memory (we COPY, we don't take ownership)
    Image(int w, int h, const byte* src)
        : Image(w, h)
    {
        if (buffer && src) {
            std::copy(src, src + GetSize(), buffer);
        }
    }

    // Deep copy ctor
    Image(const Image& other)
        : width(other.width), height(other.height),
          buffer(other.buffer ? new byte[other.GetSize()] : nullptr)
    {
        if (buffer) {
            std::copy(other.buffer, other.buffer + other.GetSize(), buffer);
        }
    }

    // Copy assignment (copy-and-swap: strong exception safety)
    Image& operator=(const Image& other) {
        if (this == &other) return *this;
        Image tmp(other);
        swap(tmp);
        return *this;
    }

    // Move ctor
    Image(Image&& other) noexcept
        : width(other.width), height(other.height), buffer(other.buffer)
    {
        other.width = other.height = 0;
        other.buffer = nullptr;
    }

    // Move assignment
    Image& operator=(Image&& other) noexcept {
        if (this == &other) return *this;
        delete[] buffer;
        width  = other.width;
        height = other.height;
        buffer = other.buffer;
        other.width = other.height = 0;
        other.buffer = nullptr;
        return *this;
    }

    int GetSize() const { return width * height * 3; }

    // x,y in [0,1]; clamp to valid pixel
    Vec3 GetPixel(int ix, int iy) const {
        //if (!buffer || width <= 0 || height <= 0) return Vec3(0,0,0);
        if (ix < 0) ix = 0; else if (ix >= width)  ix = width  - 1;
        if (iy < 0) iy = 0; else if (iy >= height) iy = height - 1;
        int index = (iy * width + ix) * 3;
        return Vec3(buffer[index] / 255.f,
                    buffer[index + 1] / 255.f,
                    buffer[index + 2] / 255.f);
    }

    void swap(Image& other) noexcept {
        using std::swap;
        swap(width, other.width);
        swap(height, other.height);
        swap(buffer, other.buffer);
    }

    ~Image() { delete[] buffer; }
};