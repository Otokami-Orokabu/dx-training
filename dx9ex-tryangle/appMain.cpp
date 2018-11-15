#include "GDX9ex.h"

// 仮想フルスクリーンへの解像度変更
bool EnterVirtualFullScreen(int width, int height)
{
    DEVMODE devMode = { 0 };
    int nModeIndex = 0;
    while (EnumDisplaySettings(NULL, nModeIndex++, &devMode))
    {
        // 一致する解像度？
        if (devMode.dmPelsWidth != width || devMode.dmPelsHeight != height)
        {
            continue;
        }
        if (devMode.dmBitsPerPel != 32)
        {
            continue;
        }
        if (devMode.dmDisplayFrequency != 60)
        {
            continue;
        }
        if (ChangeDisplaySettings(&devMode, CDS_TEST) == DISP_CHANGE_SUCCESSFUL)
        {
            // 力変え出来そうな設定を採用する
            ChangeDisplaySettings(&devMode, CDS_FULLSCREEN);
            return true;
        }
    }
    return false;
}

// 仮想フルスクリーンへの解像度変更解除
void LeaveVirtualFullScreen()
{
    ChangeDisplaySettings(NULL, 0);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int)
{
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif // defined(DEBUG) | defined(_DEBUG)

    App::ScreenMode screenMode = App::ScreenMode::WindowMode;
    if (App::ScreenMode::VirtualFullScreenMode == screenMode)
    {
        EnterVirtualFullScreen(App::WindowWidth, App::WindowHeight);
    }


    win32 win;
    HWND hWnd = win.SetupWindow(App::WindowWidth, App::WindowHeight, screenMode);
    if (!hWnd)
        return -1;

    HRESULT hr = S_OK;
    GDX9ex dx9ex;
    hr = dx9ex.Initialize(hWnd, App::WindowWidth, App::WindowHeight, screenMode);
    if (FAILED(hr))
        return -1;

    UpdateWindow(hWnd);
    ShowWindow(hWnd, SW_SHOW);
 
    bool finish = false;
    MSG msg = { 0 };
    do
    {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if(msg.message == WM_QUIT)
            {
                finish = true;
                break;
            }
        }

        dx9ex.Render();

    } while (!finish);

    if (App::ScreenMode::VirtualFullScreenMode == screenMode)
    {
        LeaveVirtualFullScreen();
    }

    return 0;
}
