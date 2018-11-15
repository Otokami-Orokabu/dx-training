#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <tchar.h>

#include <wrl/client.h>

#include <d3d9.h>
#include <d3dcompiler.h>

#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dcompiler.lib")

namespace App
{
    const int WindowWidth(800);
    const int WindowHeight(600);

    const std::wstring window_class(L"ororo");
    const std::wstring window_name(L"dx9ex-demo");
}
