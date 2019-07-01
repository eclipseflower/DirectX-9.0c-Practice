#pragma once

#include "d3dUtil.h"

class Water
{
public:
	Water(int m, int n, float dx, float dz, const D3DXMATRIX& toWorld);
	~Water();

	DWORD GetNumTriangles();
	DWORD GetNumVertices();

	void OnLostDevice();
	void OnResetDevice();

	void Update(float dt);
	void Draw();

private:
	void BuildFX();

private:
	ID3DXMesh* mMesh;
	D3DXMATRIX mToWorld;
	ID3DXEffect* mFX;

	DWORD mVertRows;
	DWORD mVertCols;

	float mWidth;
	float mDepth;

	float mDX;
	float mDZ;

	D3DXHANDLE mhTech;
	D3DXHANDLE mhWVP;
	D3DXHANDLE mhWorld;
	D3DXHANDLE mhEyePosW;
};

