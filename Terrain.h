#pragma once

#include "Heightmap.h"
#include "d3dUtil.h"
#include "Vertex.h"

class Terrain
{
public:
	Terrain(UINT vertRows, UINT vertCols, float dx, float dz,
		std::string heightmap, std::string tex0, std::string tex1,
		std::string tex2, std::string blendMap, float heightScale,
		float yOffset);
	~Terrain();

	DWORD GetNumTriangles();
	DWORD GetNumVertices();

	float GetWidth();
	float GetDepth();

	void OnLostDevice();
	void OnResetDevice();

	// (x, z) relative to terrain's local space.
	float GetHeight(float x, float z);

	void SetDirToSunW(const D3DXVECTOR3& d);

	void Draw();

private:
	void BuildGeometry();
	void BuildSubGridMesh(RECT& R, VertexPNT* gridVerts);
	void BuildEffect();

	struct SubGrid
	{
		ID3DXMesh *mesh;
		AABB box;
		bool operator < ( SubGrid& rhs);

		const static int NUM_ROWS = 33;
		const static int NUM_COLS = 33;
		const static int NUM_TRIS = (NUM_ROWS - 1)*(NUM_COLS - 1) * 2;
		const static int NUM_VERTS = NUM_ROWS * NUM_COLS;
	};

private:
	Heightmap mHeightmap;
	std::vector<SubGrid> mSubGrids;

	DWORD mVertRows;
	DWORD mVertCols;

	float mWidth;
	float mDepth;

	float mDX;
	float mDZ;

	IDirect3DTexture9* mTex0;
	IDirect3DTexture9* mTex1;
	IDirect3DTexture9* mTex2;
	IDirect3DTexture9* mBlendMap;
	ID3DXEffect*       mFX;
	D3DXHANDLE         mhTech;
	D3DXHANDLE         mhViewProj;
	D3DXHANDLE         mhDirToSunW;
	D3DXHANDLE         mhTex0;
	D3DXHANDLE         mhTex1;
	D3DXHANDLE         mhTex2;
	D3DXHANDLE         mhBlendMap;
};

