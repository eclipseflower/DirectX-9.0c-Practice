#pragma once
#include "d3dUtil.h"

class WaterDMap
{
public:
	struct InitInfo
	{
		DirLight dirLight;
		Mtrl     mtrl;
		std::string fxFilename;
		int      vertRows;
		int      vertCols;
		float    dx;
		float    dz;
		std::string waveMapFilename0;
		std::string waveMapFilename1;
		std::string dmapFilename0;
		std::string dmapFilename1;
		D3DXVECTOR2 waveNMapVelocity0;
		D3DXVECTOR2 waveNMapVelocity1;
		D3DXVECTOR2 waveDMapVelocity0;
		D3DXVECTOR2 waveDMapVelocity1;
		D3DXVECTOR2 scaleHeights;
		float texScale;
		D3DXMATRIX toWorld;
	};

public:
	WaterDMap(InitInfo& initInfo);

	~WaterDMap();

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
	ID3DXEffect* mFX;

	// The two normal maps to scroll.
	IDirect3DTexture9* mWaveMap0;
	IDirect3DTexture9* mWaveMap1;

	// The two displacement maps to scroll.
	IDirect3DTexture9* mDispMap0;
	IDirect3DTexture9* mDispMap1;

	// Offset of normal maps for scrolling (vary as a function of time)
	D3DXVECTOR2 mWaveNMapOffset0;
	D3DXVECTOR2 mWaveNMapOffset1;

	// Offset of displacement maps for scrolling (vary as a function of time)
	D3DXVECTOR2 mWaveDMapOffset0;
	D3DXVECTOR2 mWaveDMapOffset1;

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
	D3DXHANDLE mhWaveNMapOffset0;
	D3DXHANDLE mhWaveNMapOffset1;
	D3DXHANDLE mhWaveDMapOffset0;
	D3DXHANDLE mhWaveDMapOffset1;
	D3DXHANDLE mhWaveDispMap0;
	D3DXHANDLE mhWaveDispMap1;
	D3DXHANDLE mhScaleHeights;
	D3DXHANDLE mhGridStepSizeL;
};

