#pragma once

#include "win32app.h"

#include <d3d9.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
//#include <d3dcompiler.h>
#pragma comment(lib,"d3d9.lib")

class GDX9ex
{
public:
    GDX9ex();
    ~GDX9ex();

    HRESULT Initialize(HWND hWnd,int width,int height, App::ScreenMode mode);
    void Render();
private:
    Microsoft::WRL::ComPtr<IDirect3D9Ex> m_d3d9;
    Microsoft::WRL::ComPtr<IDirect3DDevice9Ex> m_d3dDev;
    D3DPRESENT_PARAMETERS m_d3dpp;
    D3DDISPLAYMODEEX m_d3ddm;
private:
    void SetupVertexBuffer();
    void SetupVertexDeclaration();
    void LoadShader();
    struct MyVertex
    {
        DirectX::XMFLOAT3 Pos;
        DirectX::PackedVector::XMCOLOR Color;
    };
    Microsoft::WRL::ComPtr<IDirect3DVertexDeclaration9> m_Declaration;
    Microsoft::WRL::ComPtr<IDirect3DVertexBuffer9> m_VertexBuffer;
    Microsoft::WRL::ComPtr<IDirect3DVertexShader9> m_VertexShader;
    Microsoft::WRL::ComPtr<IDirect3DPixelShader9> m_PixelShader;
    
    DirectX::XMMATRIX m_mtxView;    // ビュー行列
    DirectX::XMMATRIX m_mtxProj;    // プロジェクション行列
};
