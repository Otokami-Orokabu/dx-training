
#include "main.cpp"

static const float quadVerts[4 * 5] =
{
  1.0f,-1.0f,0.0f,1.0f,0.0f,
 -1.0f,-1.0f,0.0f,0.0f,0.0f,
  1.0f, 1.0f,0.0f,1.0f,1.0f,
 -1.0f, 1.0f,0.0f,0.0f,1.0f
};

static const char shaderCode[] = \
"float4 ps_main(float2 u:texcoord):color"
"{"
"float d = distance(float2(0.5,0.5), u);"
"return step(a,d);"
"}";

namespace dx9ex
{
    using namespace Microsoft::WRL;
    ComPtr<IDirect3D9Ex> d3d(nullptr);
    ComPtr<IDirect3DDevice9Ex> device(nullptr);
    ComPtr<IDirect3DPixelShader9> m_PixelShader(nullptr);
    static D3DPRESENT_PARAMETERS d3dpp = {
    App::WindowWidth, App::WindowHeight,D3DFMT_A8R8G8B8,  0, D3DMULTISAMPLE_NONE,
    0, D3DSWAPEFFECT_DISCARD, 0, TRUE, FALSE,
    D3DFMT_D24S8, 0, 0, D3DPRESENT_INTERVAL_IMMEDIATE };
}

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

// デバッグ用エントリ
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    WNDCLASSEX wcex = { 0 };
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.hInstance = GetModuleHandle(nullptr);
    wcex.lpfnWndProc = WndProc;
    wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcex.hIconSm = wcex.hIcon;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
    wcex.lpszClassName = App::window_class.c_str();
    if (!::RegisterClassEx(&wcex))
        return -1;

    RECT r{ 0,0, App::WindowWidth,App::WindowHeight };

    HWND hWnd = CreateWindow(
        wcex.lpszClassName,
        App::window_name.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        r.right - r.left,
        r.bottom - r.top,
        nullptr,
        nullptr,
        wcex.hInstance,
        nullptr
    );
    if (!hWnd)
        return -1;

    if (Direct3DCreate9Ex(D3D_SDK_VERSION, dx9ex::d3d.GetAddressOf()) < 0)
        return -1;

    dx9ex::d3dpp.hDeviceWindow = hWnd;

    if (dx9ex::d3d->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, dx9ex::d3dpp.hDeviceWindow, D3DCREATE_HARDWARE_VERTEXPROCESSING, &dx9ex::d3dpp, nullptr, dx9ex::device.GetAddressOf()) < 0)
        return -1;

    // シェーダーコンパイル

    Microsoft::WRL::ComPtr< ID3DBlob> tmp;
    if (D3DCompile(shaderCode, sizeof(shaderCode) - 1, nullptr, nullptr, nullptr, "ps_main", "ps_2_0", D3DCOMPILE_OPTIMIZATION_LEVEL3 | D3DCOMPILE_PREFER_FLOW_CONTROL, 0, tmp.GetAddressOf(), nullptr) < 0)
        return -1;

    // シェーダをセット.
    if (dx9ex::device->CreatePixelShader(reinterpret_cast<DWORD*>(tmp->GetBufferPointer()), dx9ex::m_PixelShader.GetAddressOf()) < 0)
        return -1;

    ShowWindow(hWnd, SW_SHOW);

    bool done = false;
    MSG msg = { 0 };
    do
    {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
            {
                done = true;
                break;
            }
        }

        dx9ex::device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 0x000000, 1.0f, 0);
        dx9ex::device->BeginScene();
        dx9ex::device->SetPixelShader(dx9ex::m_PixelShader.Get());
        dx9ex::device->SetFVF(D3DFVF_XYZ | D3DFVF_TEX1);
        dx9ex::device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quadVerts, 5 * sizeof(float));

        dx9ex::device->EndScene();
        dx9ex::device->Present(nullptr, nullptr, nullptr, nullptr);

    } while (!done);

    return 0;
}
