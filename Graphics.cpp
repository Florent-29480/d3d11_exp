#include "Graphics.h"
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <cstring>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

namespace
{
	void ThrowIfFailed(HRESULT hr, int line)
	{
		if (FAILED(hr))
		{
			std::ostringstream oss;
			oss << "Erreur Direct3D 0x" << std::hex << static_cast<unsigned long>(hr)
				<< std::dec << " (Graphics.cpp, ligne " << line << ")";
			throw std::runtime_error(oss.str());
		}
	}

	Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(const char* src, const char* target)
	{
		Microsoft::WRL::ComPtr<ID3DBlob> blob;
		Microsoft::WRL::ComPtr<ID3DBlob> errors;
		const HRESULT hr = D3DCompile(
			src, std::strlen(src), nullptr, nullptr, nullptr,
			"main", target, 0, 0, blob.GetAddressOf(), errors.GetAddressOf());
		if (FAILED(hr))
		{
			std::string msg = errors
				? std::string(static_cast<const char*>(errors->GetBufferPointer()), errors->GetBufferSize())
				: "Echec de compilation du shader";
			throw std::runtime_error(msg);
		}
		return blob;
	}

	// Shaders 

	// Triangle 
	constexpr const char* triVertexShaderSrc = R"(
struct VSOut
{
	float4 color : Color;
	float4 pos   : SV_Position;
};

VSOut main(float2 pos : Position, float4 color : Color)
{
	VSOut o;
	o.pos = float4(pos, 0.0f, 1.0f);
	o.color = color;
	return o;
}
)";

	// Tétraèdre 
	constexpr const char* tetraVertexShaderSrc = R"(
cbuffer CBuf
{
	matrix transform;
};

struct VSOut
{
	float4 color : Color;
	float4 pos   : SV_Position;
};

VSOut main(float3 pos : Position, float4 color : Color)
{
	VSOut o;
	o.pos = mul(float4(pos, 1.0f), transform);
	o.color = color;
	return o;
}
)";

	// Couleur
	constexpr const char* pixelShaderSrc = R"(
float4 main(float4 color : Color) : SV_Target
{
	return color;
}
)";

	// ---------- Données de vertex ----------

	struct Color4 { unsigned char r, g, b, a; };
	struct Float2 { float x, y; };
	struct Float3 { float x, y, z; };

	struct TriVertex
	{
		Float2 pos;
		Color4 color;
	};

	struct TetraVertex
	{
		Float3 pos;
		Color4 color;
	};

	struct ConstantBufferData
	{
		DirectX::XMFLOAT4X4 transform;
	};
}

#define GFX_CHECK(hrcall) ThrowIfFailed((hrcall), __LINE__)

Graphics::Graphics(HWND hWnd)
{
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 0;
	sd.BufferDesc.RefreshRate.Denominator = 0;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	sd.OutputWindow = hWnd;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;

	GFX_CHECK(D3D11CreateDeviceAndSwapChain(
		nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
		nullptr, 0, D3D11_SDK_VERSION,
		&sd, pSwap.GetAddressOf(), pDevice.GetAddressOf(), nullptr, pContext.GetAddressOf()));

	ComPtr<ID3D11Texture2D> pBackBuffer;
	GFX_CHECK(pSwap->GetBuffer(0, IID_PPV_ARGS(pBackBuffer.GetAddressOf())));
	D3D11_TEXTURE2D_DESC td = {};
	pBackBuffer->GetDesc(&td);
	width = td.Width;
	height = td.Height;
	GFX_CHECK(pDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, pTarget.GetAddressOf()));

	// Pixel shader commun
	const auto psBlob = CompileShader(pixelShaderSrc, "ps_5_0");
	GFX_CHECK(pDevice->CreatePixelShader(
		psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, pPixelShader.GetAddressOf()));

	InitTriangle();
	InitTetrahedron();
}

void Graphics::InitTriangle()
{
	// Sens horaire = face avant (convention D3D par défaut)
	const TriVertex vertices[] =
	{
		{ {  0.0f,  0.5f }, { 255,   0,   0, 255 } }, // haut, rouge
		{ {  0.5f, -0.5f }, {   0, 255,   0, 255 } }, // bas droite, vert
		{ { -0.5f, -0.5f }, {   0,   0, 255, 255 } }, // bas gauche, bleu
	};

	D3D11_BUFFER_DESC bd = {};
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(vertices);
	bd.StructureByteStride = sizeof(TriVertex);
	D3D11_SUBRESOURCE_DATA sd = {};
	sd.pSysMem = vertices;
	GFX_CHECK(pDevice->CreateBuffer(&bd, &sd, pTriVertexBuffer.GetAddressOf()));

	const auto vsBlob = CompileShader(triVertexShaderSrc, "vs_5_0");
	GFX_CHECK(pDevice->CreateVertexShader(
		vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, pTriVertexShader.GetAddressOf()));

	const D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{ "Position", 0, DXGI_FORMAT_R32G32_FLOAT,   0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "Color",    0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	GFX_CHECK(pDevice->CreateInputLayout(
		ied, static_cast<UINT>(std::size(ied)),
		vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), pTriInputLayout.GetAddressOf()));
}

void Graphics::InitTetrahedron()
{
	// Tétraèdre
	const Float3 A{ 1.0f,  1.0f,  1.0f };
	const Float3 B{ 1.0f, -1.0f, -1.0f };
	const Float3 C{ -1.0f,  1.0f, -1.0f };
	const Float3 D{ -1.0f, -1.0f,  1.0f };
	const Color4 red{ 255,   0,   0, 255 };
	const Color4 green{ 0, 255,   0, 255 };
	const Color4 blue{ 0,   0, 255, 255 };
	const Color4 yellow{ 255, 220, 0, 255 };

	const TetraVertex vertices[] =
	{
		{ A, red    }, { B, red    }, { C, red    },
		{ A, green  }, { D, green  }, { B, green  },
		{ A, blue   }, { C, blue   }, { D, blue   },
		{ B, yellow }, { D, yellow }, { C, yellow },
	};

	D3D11_BUFFER_DESC bd = {};
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(vertices);
	bd.StructureByteStride = sizeof(TetraVertex);
	D3D11_SUBRESOURCE_DATA sd = {};
	sd.pSysMem = vertices;
	GFX_CHECK(pDevice->CreateBuffer(&bd, &sd, pTetraVertexBuffer.GetAddressOf()));

	// Constant buffer : réécrit à chaque frame par le CPU
	D3D11_BUFFER_DESC cbd = {};
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbd.ByteWidth = sizeof(ConstantBufferData); // 64 octets, multiple de 16
	GFX_CHECK(pDevice->CreateBuffer(&cbd, nullptr, pConstantBuffer.GetAddressOf()));

	const auto vsBlob = CompileShader(tetraVertexShaderSrc, "vs_5_0");
	GFX_CHECK(pDevice->CreateVertexShader(
		vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, pTetraVertexShader.GetAddressOf()));

	const D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{ "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "Color",    0, DXGI_FORMAT_R8G8B8A8_UNORM,  0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	GFX_CHECK(pDevice->CreateInputLayout(
		ied, static_cast<UINT>(std::size(ied)),
		vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), pTetraInputLayout.GetAddressOf()));
}

void Graphics::BindCommonState()
{
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pContext->PSSetShader(pPixelShader.Get(), nullptr, 0u);

	// Rasterizer
	D3D11_VIEWPORT vp = {};
	vp.Width = static_cast<float>(width);
	vp.Height = static_cast<float>(height);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	pContext->RSSetViewports(1u, &vp);

	// Output Merger (pas de depth buffer)
	pContext->OMSetRenderTargets(1u, pTarget.GetAddressOf(), nullptr);
}

void Graphics::DrawTriangle()
{
	const UINT stride = sizeof(TriVertex);
	const UINT offset = 0u;
	pContext->IASetVertexBuffers(0u, 1u, pTriVertexBuffer.GetAddressOf(), &stride, &offset);
	pContext->IASetInputLayout(pTriInputLayout.Get());
	pContext->VSSetShader(pTriVertexShader.Get(), nullptr, 0u);
	BindCommonState();

	pContext->Draw(3u, 0u);
}

void Graphics::DrawTetrahedron(float yaw, float pitch)
{
	using namespace DirectX;

	// Matrice modèle -> écran : rotation, recul devant la caméra, projection perspective
	const float aspect = static_cast<float>(width) / static_cast<float>(height);
	const XMMATRIX m =
		XMMatrixRotationRollPitchYaw(pitch, yaw, 0.0f) *
		XMMatrixTranslation(0.0f, 0.0f, 6.0f) *
		XMMatrixPerspectiveFovLH(XM_PIDIV4, aspect, 0.5f, 20.0f);

	// DirectXMath est row-major, HLSL lit en column-major -> transposer à l'envoi
	ConstantBufferData cb;
	XMStoreFloat4x4(&cb.transform, XMMatrixTranspose(m));

	D3D11_MAPPED_SUBRESOURCE msr = {};
	GFX_CHECK(pContext->Map(pConstantBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &msr));
	std::memcpy(msr.pData, &cb, sizeof(cb));
	pContext->Unmap(pConstantBuffer.Get(), 0u);

	const UINT stride = sizeof(TetraVertex);
	const UINT offset = 0u;
	pContext->IASetVertexBuffers(0u, 1u, pTetraVertexBuffer.GetAddressOf(), &stride, &offset);
	pContext->IASetInputLayout(pTetraInputLayout.Get());
	pContext->VSSetShader(pTetraVertexShader.Get(), nullptr, 0u);
	pContext->VSSetConstantBuffers(0u, 1u, pConstantBuffer.GetAddressOf());
	BindCommonState();

	pContext->Draw(12u, 0u);
}

void Graphics::ClearBuffer(float red, float green, float blue) noexcept
{
	const float color[] = { red, green, blue, 1.0f };
	pContext->ClearRenderTargetView(pTarget.Get(), color);
}

void Graphics::EndFrame()
{
	GFX_CHECK(pSwap->Present(1u, 0u));
}