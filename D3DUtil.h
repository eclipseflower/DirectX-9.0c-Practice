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
class Camera;
extern Camera* gCamera;

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

float GetRandomFloat(float a, float b);
void GetRandomVec(D3DXVECTOR3& out);

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

struct AABB
{
	// Initialize to an infinitely small bounding box.
	AABB()
		: minPt(INFINITY, INFINITY, INFINITY),
		maxPt(-INFINITY, -INFINITY, -INFINITY) {}

	D3DXVECTOR3 Center()const
	{
		return (minPt + maxPt)*0.5f;
	}

	D3DXVECTOR3 Extent()const
	{
		return (maxPt - minPt)*0.5f;
	}

	void Xform(const D3DXMATRIX& M, AABB& out)
	{
		// Convert to center/extent representation.
		D3DXVECTOR3 c = Center();
		D3DXVECTOR3 e = Extent();

		// Transform center in usual way.
		D3DXVec3TransformCoord(&c, &c, &M);

		// Transform extent.
		D3DXMATRIX absM;
		D3DXMatrixIdentity(&absM);
		absM(0, 0) = fabsf(M(0, 0)); absM(0, 1) = fabsf(M(0, 1)); absM(0, 2) = fabsf(M(0, 2));
		absM(1, 0) = fabsf(M(1, 0)); absM(1, 1) = fabsf(M(1, 1)); absM(1, 2) = fabsf(M(1, 2));
		absM(2, 0) = fabsf(M(2, 0)); absM(2, 1) = fabsf(M(2, 1)); absM(2, 2) = fabsf(M(2, 2));
		D3DXVec3TransformNormal(&e, &e, &absM);

		// Convert back to AABB representation.
		out.minPt = c - e;
		out.maxPt = c + e;
	}

	D3DXVECTOR3 minPt;
	D3DXVECTOR3 maxPt;
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