#pragma once

#if defined(DEBUG) | defined(_DEBUG)
#ifndef D3D_DEBUG_INFO
#define D3D_DEBUG_INFO
#endif
#endif

#include <d3d9.h>
#include <d3dx9.h>
#include <DxErr.h>
#include <string>
#include <sstream>
#include <vector>

class D3DApp;
struct Mtrl;
extern D3DApp* gD3dApp;
extern IDirect3DDevice9* gD3dDevice;

void GenTriGrid(int numVertRows, int numVertCols, float dx, float dz, const D3DXVECTOR3& center,
	std::vector<D3DXVECTOR3>& verts, std::vector<DWORD>& indices);

void LoadXFile(const std::string& filename, ID3DXMesh** meshOut, std::vector<Mtrl>& mtrls, 
	std::vector<IDirect3DTexture9*>& texs);

const D3DXCOLOR _WHITE(1.0f, 1.0f, 1.0f, 1.0f);
const D3DXCOLOR _BLACK(0.0f, 0.0f, 0.0f, 1.0f);
const D3DXCOLOR _RED(1.0f, 0.0f, 0.0f, 1.0f);
const D3DXCOLOR _GREEN(0.0f, 1.0f, 0.0f, 1.0f);
const D3DXCOLOR _BLUE(0.0f, 0.0f, 1.0f, 1.0f);

const float EPSILON = 0.001f;

struct Mtrl
{
	Mtrl() : ambient(_WHITE), diffuse(_WHITE), spec(_WHITE), specPower(8.0f) {}
	Mtrl(const D3DXCOLOR& a, const D3DXCOLOR& d, const D3DXCOLOR& s, float power) : ambient(a), diffuse(d), spec(s), specPower(power) {}

	D3DXCOLOR ambient;
	D3DXCOLOR diffuse;
	D3DXCOLOR spec;
	float specPower;
};

struct DirLight
{
	D3DXCOLOR ambient;
	D3DXCOLOR diffuse;
	D3DXCOLOR spec;
	D3DXVECTOR3 dirW;
};

#define ReleaseCOM(x) { if(x){ x->Release();x = 0; } }

#if defined(DEBUG) | defined(_DEBUG)
#ifndef HR
#define HR(x)                                      \
	{                                                  \
		HRESULT hr = x;                                \
		if(FAILED(hr))                                 \
		{                                              \
			DXTrace(__FILE__, __LINE__, hr, #x, TRUE); \
		}                                              \
	}
#endif

#else
#ifndef HR
#define HR(x) x;
#endif
#endif