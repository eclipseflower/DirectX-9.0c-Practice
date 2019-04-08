#pragma once
#include "GfxStats.h"
#include "D3DApp.h"
class CloudDemoApp : public D3DApp
{
public:
	CloudDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~CloudDemoApp();

	bool CheckDeviceCaps();
	void OnLostDevice();
	void OnResetDevice();
	void UpdateScene(float dt);
	void DrawScene();

	// Helper methods
	void BuildGridGeometry();
	void BuildFx();
	void BuildViewMtx();
	void BuildProjMtx();

private:
	GfxStats* mGfxStats;

	DWORD mNumGridVertices;
	DWORD mNumGridTriangles;

	IDirect3DVertexBuffer9* mGridVB;
	IDirect3DIndexBuffer9*  mGridIB;
	IDirect3DTexture9*      mCloudTex0;
	IDirect3DTexture9*      mCloudTex1;

	ID3DXEffect* mFX;
	D3DXHANDLE   mhTech;
	D3DXHANDLE   mhWVP;
	D3DXHANDLE   mhCloudTex0;
	D3DXHANDLE   mhCloudTex1;
	D3DXHANDLE   mhTexOffset0;
	D3DXHANDLE   mhTexOffset1;

	D3DXVECTOR2 mTexOffset0;
	D3DXVECTOR2 mTexOffset1;

	float mCameraRotationY;
	float mCameraRadius;
	float mCameraHeight;

	D3DXMATRIX mWorld;
	D3DXMATRIX mView;
	D3DXMATRIX mProj;
};

