#include "GDX9ex.h"


using namespace DirectX;
using namespace DirectX::PackedVector;

namespace
{
    std::wstring GetExecutionDirectory()
    {
        wchar_t filePath[MAX_PATH];
        GetModuleFileNameW(NULL, filePath, sizeof(filePath));
        wchar_t* p = wcsrchr(filePath, L'\\');
        std::wstring strPath(filePath, p);
        return strPath;
    }

    bool LoadBinaryFile(const std::wstring& fileName,std::vector<uint8_t> & loadBuf)
    {
        loadBuf.clear();
        std::wstring path = GetExecutionDirectory();
        path += std::wstring(L"\\");
        path += fileName;
        std::ifstream infile(path, std::ifstream::binary);
        if (!infile)
        {
            return false;
        }
        int size = static_cast<int>(infile.seekg(0, std::ifstream::end).tellg());
        loadBuf.resize(size);
        infile.seekg(0, std::ifstream::beg);
        infile.read(reinterpret_cast<char*>(loadBuf.data()), size);
        return true;
    }
}

GDX9ex::GDX9ex() :
    m_d3d9(nullptr),
    m_d3dDev(nullptr),
    m_Declaration(nullptr),
    m_VertexBuffer(nullptr),
    m_VertexShader(nullptr),
    m_PixelShader(nullptr)
{
    memset(&m_d3dpp, 0x00, sizeof(m_d3dpp));
    memset(&m_d3ddm, 0x00, sizeof(m_d3ddm));
}

GDX9ex::~GDX9ex()
{
}

HRESULT GDX9ex::Initialize(HWND hWnd, int width, int height, App::ScreenMode mode)
{
    HRESULT hr = S_OK;

    try
    {
        hr = Direct3DCreate9Ex(D3D_SDK_VERSION, m_d3d9.GetAddressOf());
        if (FAILED(hr))
            throw std::runtime_error("failed Direct3DCreate9Ex");

        //
        m_d3dpp.BackBufferWidth = width;                // 描画領域の幅.
        m_d3dpp.BackBufferHeight = height;              // 描画領域の高さ.
        m_d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;     // フォーマット指定.
        m_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;     // バックバッファのスワップエフェクト.
        m_d3dpp.BackBufferCount = 2;                    // バックバッファ数.
        m_d3dpp.EnableAutoDepthStencil = TRUE;          // 深度バッファ.
        m_d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;  // 深度バッファのフォーマット.
        m_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;   // vSync待ちを行わない.
        m_d3dpp.hDeviceWindow = hWnd;

        // 
        m_d3ddm.Size = sizeof(D3DDISPLAYMODEEX);
        m_d3ddm.Format = m_d3dpp.BackBufferFormat;
        m_d3ddm.Width = m_d3dpp.BackBufferWidth;
        m_d3ddm.Height = m_d3dpp.BackBufferHeight;
        m_d3ddm.ScanLineOrdering = D3DSCANLINEORDERING_UNKNOWN;

        D3DDISPLAYMODEEX* dm = nullptr;

        switch (mode)
        {
        case App::WindowMode:
        case App::VirtualFullScreenMode:
            // ウインドウモード
            m_d3dpp.Windowed = TRUE;
            break;
        case App::FullScreenMode:
            // フルスクリーン
            m_d3dpp.Windowed = FALSE;
            m_d3dpp.FullScreen_RefreshRateInHz = 60;
            m_d3ddm.RefreshRate = m_d3dpp.FullScreen_RefreshRateInHz;
            dm = &m_d3ddm;
            break;
        default:
            throw std::runtime_error("Not found ScreenType");
        }

        hr = m_d3d9->CreateDeviceEx(
            D3DADAPTER_DEFAULT,
            D3DDEVTYPE_HAL,
            hWnd,
            D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE,
            &m_d3dpp,
            dm,
            m_d3dDev.GetAddressOf()
        );

        if (FAILED(hr))
            throw std::runtime_error("failed CreateDeviceEx");

        // 頂点宣言の準備
        SetupVertexDeclaration();

         // 頂点バッファの準備
        SetupVertexBuffer();

        // シェーダーファイルのロードと生成
        LoadShader();

        // ビュー行列とプロジェクション行列をセットアップ.

        // カメラ(視点)の情報
        XMFLOAT3 eyePos(0.0f, 2.0f, -4.0f);
        XMFLOAT3 eyeTarget(0.0f, 0.0f, 0.0f);
        XMFLOAT3 eyeUp(0.0f, 1.0f, 0.0f);
        m_mtxView = XMMatrixLookAtLH(
            XMLoadFloat3(&eyePos),
            XMLoadFloat3(&eyeTarget),
            XMLoadFloat3(&eyeUp)
        );

        // プロジェクション行列
        float fov = XMConvertToRadians(45.f);
        float aspect = static_cast<float>(width) / static_cast<float>(height);
        float nearZ = 0.1f;
        float farZ = 100.0f;
        m_mtxProj = XMMatrixPerspectiveFovLH(
            fov,
            aspect,
            nearZ,
            farZ
        );

        // ビューポートの設定
        D3DVIEWPORT9 vp;
        vp.X = 0;
        vp.Y = 0;
        vp.Width = width;
        vp.Height = height;
        vp.MinZ = 0.0f;
        vp.MaxZ = 1.0f;
        hr = m_d3dDev->SetViewport(&vp);
    }
    catch (std::runtime_error e)
    {
        MessageBoxA(hWnd, e.what(), "初期化失敗", MB_OK);
    }

    return hr;
}

void GDX9ex::Render()
{
    // 画面クリア
    m_d3dDev->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, D3DCOLOR_RGBA(0x40, 0x80, 0xFF, 0x00), 1.0f, 0);
    m_d3dDev->BeginScene();
    // ワールド行列は単位行列
    XMMATRIX world = XMMatrixIdentity();
    // ここでワールド行列に回転や平行移動を作用させると三角形が動く.
    //world = XMMatrixRotationAxis(XMVectorSet(0, 1, 0, 0), XMConvertToRadians(45.0f));

    // ワールド、ビュー、プロジェクションを結合した行列を求める
    XMFLOAT4X4 mtxWVP;
    XMStoreFloat4x4(&mtxWVP, XMMatrixTranspose(world * m_mtxView * m_mtxProj));
    m_d3dDev->SetVertexShaderConstantF(0, &mtxWVP.m[0][0], 4);

    m_d3dDev->SetVertexDeclaration(m_Declaration.Get());
    m_d3dDev->SetVertexShader(m_VertexShader.Get());
    m_d3dDev->SetPixelShader(m_PixelShader.Get());
    m_d3dDev->SetStreamSource(0, m_VertexBuffer.Get(), 0, sizeof(MyVertex));

    m_d3dDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    m_d3dDev->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1);

    m_d3dDev->EndScene();
    m_d3dDev->PresentEx(nullptr, nullptr, nullptr, nullptr, 0);
}

// 頂点宣言の作成・準備を行います.
void GDX9ex::SetupVertexDeclaration()
{
    HRESULT hr = S_OK;
    D3DVERTEXELEMENT9 decls[] =
    {
        //Stream, Offset, Type , Method, Usage ,UsageIndex
        { 0,  0, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
        { 0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,0 },
        D3DDECL_END(),
    };

    hr = m_d3dDev->CreateVertexDeclaration(decls, m_Declaration.GetAddressOf());
    if (FAILED(hr))
        throw std::runtime_error("Failed CreateVertexDeclaration");
}

// 頂点バッファの作成・準備
void GDX9ex::SetupVertexBuffer()
{
    const MyVertex vertices[] = {
        { XMFLOAT3(0.0f, 0.0f, 0.0f), XMCOLOR(0xff00ff00) },
        { XMFLOAT3(1.0f, 1.0f, 0.0f), XMCOLOR(0xffff0000) },
        { XMFLOAT3(1.0f, 0.0f, 0.0f), XMCOLOR(0xff0000ff) },
    };

    HRESULT hr = S_OK;
    hr = m_d3dDev->CreateVertexBuffer(
        sizeof(vertices),
        0,
        0,
        D3DPOOL_DEFAULT,
        m_VertexBuffer.GetAddressOf(),
        nullptr);
    if (FAILED(hr))
        throw std::runtime_error("Failed CreateVertexBuffer");
    void* p = nullptr;
    hr = m_VertexBuffer->Lock(0, 0, &p, 0);
    if (SUCCEEDED(hr))
    {
        // 事前に用意した頂点データをコピーする.
        memcpy_s(p, sizeof(vertices), vertices, sizeof(vertices));
        m_VertexBuffer->Unlock();
    }
}

// 
void GDX9ex::LoadShader()
{
    HRESULT hr = S_OK;
    std::vector<uint8_t> buf;
    // Vertex Shader
    if (!LoadBinaryFile(L"VertexShader.cso", buf))
        throw std::runtime_error("Failed load shader file");

    hr = m_d3dDev->CreateVertexShader(reinterpret_cast<DWORD*>(buf.data()), m_VertexShader.GetAddressOf());
    if (FAILED(hr))
        throw std::runtime_error("Failed CreateVertexShader");

    // Pixel Shader
    if (!LoadBinaryFile(L"PixelShader.cso", buf))
        throw std::runtime_error("Failed load shader file");

    hr = m_d3dDev->CreatePixelShader(reinterpret_cast<DWORD*>(buf.data()),m_PixelShader.GetAddressOf());
    if (FAILED(hr))
        throw std::runtime_error("Failed CreatePixelShader");
}
