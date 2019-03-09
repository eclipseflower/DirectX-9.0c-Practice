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
extern D3DApp* gD3dApp;
extern IDirect3DDevice9* gD3dDevice;

void GenTriGrid(int numVertRows, int numVertCols, float dx, float dz, const D3DXVECTOR3& center,
	std::vector<D3DXVECTOR3>& verts, std::vector<DWORD>& indices);

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