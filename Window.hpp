#pragma once
#include "Util.hpp"
#include "Vec.hpp"
#include "Settings.hpp"
#ifdef _WIN32
    #undef UNICODE
    #undef _UNICODE
    #include <windows.h>
    #undef min
    #undef max
#endif

namespace Graphics{
    #ifdef _WIN32
    LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        if (msg == WM_DESTROY) PostQuitMessage(0);
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    #endif
    struct Window{
        int width;
        int height;
        const char* title;
        uint frameCounter = 0;

        HWND hwnd;
        HDC hdc;

        Window(int w, int h, const char* t) : width(w), height(h), title(t) {}

        void Init(){
            #ifdef _WIN32
                WNDCLASS wc = {0};
                wc.lpfnWndProc = WndProc;
                wc.hInstance = GetModuleHandle(NULL);
                wc.lpszClassName = "MyWindowClass";
                RegisterClass(&wc);

                hwnd = CreateWindow("MyWindowClass", "Pixel Window", WS_OVERLAPPEDWINDOW,
                                        CW_USEDEFAULT, CW_USEDEFAULT, SCREEN_WIDTH, SCREEN_HEIGHT,
                                        NULL, NULL, wc.hInstance, NULL);
                ShowWindow(hwnd, SW_SHOW);

                hdc = GetDC(hwnd);

            #endif
        }
        void Pump(){
            MSG msg = {0};
            GetMessage(&msg, NULL, 0, 0);
            TranslateMessage(&msg);
            DispatchMessage(&msg);

        }
        void StartLoop(fnptr(void, loopFunction, Window*)){
            while (true) { 
                //Pump();
                loopFunction(this);
                // Set pixel at (100, 100) to red
                frameCounter++;
                if(frameCounter > 1000000)frameCounter = 0;
            }
        }
        void DrawPixel(int x, int y, Vec3 colour){
            looph(i,3){
                colour[i] = std::min(1.f,std::max(0.f,colour[i]));
            }
            SetPixel(hdc, x, y, RGB((int)(colour.x*255.f), (int)(colour.y*255.f), (int)(colour.z*255.f)));
        }
    };
}

