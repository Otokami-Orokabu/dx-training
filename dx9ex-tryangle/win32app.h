#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <fstream>


#if !defined(UINCODE) && !defined(_UNICODE)
#error This is only for Unicode. It is necssary to compiie with Unicode.
#endif // !defined(UINCODE) && !defined(_UNICODE)

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <tchar.h>

//#include <wrl.h>
#include <wrl/client.h>
#include <shellapi.h>

#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")

#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define NEW ::new( _NORMAL_BLOCK,__FILE__, __LINE__)
#else
#define NEW ::new
#endif

namespace App
{
    enum ScreenMode {
        WindowMode,
        FullScreenMode,
        VirtualFullScreenMode,
    };

    const int WindowWidth(800);
    const int WindowHeight(600);
}

class win32
{
public:
    win32();
    ~win32();

    HWND SetupWindow(int width, int height, App::ScreenMode mode);
private:
    HINSTANCE m_hInst;
    WNDCLASSEX m_wcex;
};
