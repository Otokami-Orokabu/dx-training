#include "win32app.h"

const std::wstring ClassName(L"n/a");

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_ACTIVATEAPP:
        break;
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
        {
            PostMessage(hWnd, WM_CLOSE, 0, 0);
        }
        break;
    case WM_CLOSE:
        PostMessage(hWnd, WM_DESTROY, 0, 0);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    return 0;
}

win32::win32() :
    m_hInst(nullptr)
{
    memset(&m_wcex, 0x00, sizeof(WNDCLASSEX));
}

win32::~win32()
{
}

HWND win32::SetupWindow(int width, int height, App::ScreenMode mode)
{
    m_hInst = GetModuleHandle(nullptr);
    WNDCLASSEX wcex = { 0 };
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.hInstance = m_hInst;
    wcex.lpfnWndProc = static_cast<WNDPROC>(WndProc);
    wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcex.hIconSm = wcex.hIcon;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
    wcex.lpszClassName = ClassName.c_str();

    if (!::RegisterClassEx(&wcex))
        return 0;

    int cx = CW_USEDEFAULT, cy = CW_USEDEFAULT;
    DWORD dwStyle;
    DWORD dwStyleEx = 0;
    RECT rect{ 0,0,width,height };

    switch (mode)
    {
    case App::WindowMode:
        // ウインドウSTYLE
        dwStyle = WS_OVERLAPPEDWINDOW;
        // ウインドウサイズ補正
        AdjustWindowRect(&rect, dwStyle, FALSE);
        break;
    case App::FullScreenMode:
        dwStyle = WS_POPUP;
        cx = 0;
        cy = 0;
        break;
    case App::VirtualFullScreenMode:
        dwStyleEx = WS_EX_TOPMOST;
        dwStyle = WS_POPUP;
        cx = 0;
        cy = 0;
        break;
    default:
        break;
    }

    HWND hWnd = CreateWindowEx(
        dwStyleEx,
        wcex.lpszClassName,
        L"or2",
        dwStyle,
        cx,
        cy,
        rect.right - rect.left,
        rect.bottom - rect.top,
        nullptr,
        nullptr,
        m_hInst,
        nullptr
    );

    return hWnd;
}
