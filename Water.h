#pragma once

#include "d3dUtil.h"

class Water
{
public:
	struct InitInfo
	{
		DirLight dirLight;
		Mtrl     mtrl;
		int      vertRows;
		int      vertCols;
		float    dx;
		float    dz;
		std::string waveMapFilename0;
		std::string waveMapFilename1;
		D3DXVECTOR2 waveMapVelocity0;
		D3DXVECTOR2 waveMapVelocity1;
		float texScale;
		D3DXMATRIX toWorld;
	};

public:
	Water(InitInfo& initInfo);

	~Water();

	DWORD GetNumTriangles();
	DWORD GetNumVertices();

	void OnLostDevice();
	void OnResetDevice();

	void Update(float dt);
	void Draw();

	void SetEnvMap(IDirect3DCubeTexture9* envMap);

private:
	void BuildFX();

private:
	ID3DXMesh* mMesh;
	ID3DXEffect* mFX;

	// The two normal maps to scroll.
	IDirect3DTexture9* mWaveMap0;
	IDirect3DTexture9* mWaveMap1;

	// Offset of normal maps for scrolling (vary as a function of time)
	D3DXVECTOR2 mWaveMapOffset0;
	D3DXVECTOR2 mWaveMapOffset1;

	InitInfo mInitInfo;
	float mWidth;
	float mDepth;

	D3DXHANDLE mhTech;
	D3DXHANDLE mhWVP;
	D3DXHANDLE mhWorld;
	D3DXHANDLE mhWorldInv;
	D3DXHANDLE mhLight;
	D3DXHANDLE mhMtrl;
	D3DXHANDLE mhEyePosW;
	D3DXHANDLE mhWaveMap0;
	D3DXHANDLE mhWaveMap1;
	D3DXHANDLE mhWaveMapOffset0;
	D3DXHANDLE mhWaveMapOffset1;
	D3DXHANDLE mhEnvMap;
};

