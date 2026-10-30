#pragma once
#include "ChiliWin.h"
#include <d3d11.h>
#include <wrl.h>

class Graphics
{
public:
	Graphics(HWND hWnd);
	Graphics(const Graphics&) = delete;
	Graphics& operator=(const Graphics&) = delete;
	~Graphics() = default;

	void EndFrame();
	void ClearBuffer(float red, float green, float blue) noexcept;
	// Version 1 : pipeline minimal, sans transformation
	void DrawTriangle();
	// Version 2 : tétraèdre 3D, orienté par deux angles (radians)
	void DrawTetrahedron(float yaw, float pitch);
private:
	void InitTriangle();
	void InitTetrahedron();
	void BindCommonState();
private:
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	UINT width = 0u;
	UINT height = 0u;
	ComPtr<ID3D11Device> pDevice;
	ComPtr<IDXGISwapChain> pSwap;
	ComPtr<ID3D11DeviceContext> pContext;
	ComPtr<ID3D11RenderTargetView> pTarget;

	// Commun
	ComPtr<ID3D11PixelShader> pPixelShader;

	// Triangle
	ComPtr<ID3D11Buffer> pTriVertexBuffer;
	ComPtr<ID3D11InputLayout> pTriInputLayout;
	ComPtr<ID3D11VertexShader> pTriVertexShader;

	// Tétraèdre
	ComPtr<ID3D11Buffer> pTetraVertexBuffer;
	ComPtr<ID3D11InputLayout> pTetraInputLayout;
	ComPtr<ID3D11VertexShader> pTetraVertexShader;
	ComPtr<ID3D11Buffer> pConstantBuffer;
};