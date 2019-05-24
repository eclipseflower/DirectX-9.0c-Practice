#pragma once
#include "GfxStats.h"
#include "Heightmap.h"
#include "D3DApp.h"
class BasicTerrainDemoApp : public D3DApp
{
public:
	BasicTerrainDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~BasicTerrainDemoApp();

	bool CheckDeviceCaps();
	void OnLostDevice();
	void OnResetDevice();
	void UpdateScene(float dt);
	void DrawScene();

	// Helper methods
	void BuildGridGeometry();
	void BuildFX();
	void BuildViewMtx();
	void BuildProjMtx();

private:
	GfxStats* mGfxStats;

	Heightmap mHeightmap;

	ID3DXMesh* mTerrainMesh;

	IDirect3DTexture9* mTex0;
	IDirect3DTexture9* mTex1;
	IDirect3DTexture9* mTex2;
	IDirect3DTexture9* mBlendMap;

	ID3DXEffect* mFX;
	D3DXHANDLE   mhTech;
	D3DXHANDLE   mhViewProj;
	D3DXHANDLE   mhDirToSunW;
	D3DXHANDLE   mhTex0;
	D3DXHANDLE   mhTex1;
	D3DXHANDLE   mhTex2;
	D3DXHANDLE   mhBlendMap;

	float mCameraRotationY;
	float mCameraRadius;
	float mCameraHeight;

	D3DXMATRIX mWorld;
	D3DXMATRIX mView;
	D3DXMATRIX mProj;
};

