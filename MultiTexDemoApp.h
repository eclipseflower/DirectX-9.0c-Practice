#pragma once
#include "GfxStats.h"
#include "D3DApp.h"
class MultiTexDemoApp : public D3DApp
{
public:
	MultiTexDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~MultiTexDemoApp();

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

	IDirect3DTexture9* mTex0;
	IDirect3DTexture9* mTex1;
	IDirect3DTexture9* mTex2;
	IDirect3DTexture9* mBlendMap;

	ID3DXEffect* mFX;
	D3DXHANDLE   mhTech;
	D3DXHANDLE   mhWVP;
	D3DXHANDLE   mhWorldInvTrans;
	D3DXHANDLE   mhLightVecW;
	D3DXHANDLE   mhDiffuseMtrl;
	D3DXHANDLE   mhDiffuseLight;
	D3DXHANDLE   mhAmbientMtrl;
	D3DXHANDLE   mhAmbientLight;
	D3DXHANDLE   mhSpecularMtrl;
	D3DXHANDLE   mhSpecularLight;
	D3DXHANDLE   mhSpecularPower;
	D3DXHANDLE   mhEyePos;
	D3DXHANDLE   mhWorld;
	D3DXHANDLE   mhTex0;
	D3DXHANDLE   mhTex1;
	D3DXHANDLE   mhTex2;
	D3DXHANDLE   mhBlendMap;

	D3DXVECTOR3 mLightVecW;
	D3DXCOLOR   mAmbientMtrl;
	D3DXCOLOR   mAmbientLight;
	D3DXCOLOR   mDiffuseMtrl;
	D3DXCOLOR   mDiffuseLight;
	D3DXCOLOR   mSpecularMtrl;
	D3DXCOLOR   mSpecularLight;
	float       mSpecularPower;

	float mCameraRotationY;
	float mCameraRadius;
	float mCameraHeight;

	D3DXMATRIX mWorld;
	D3DXMATRIX mView;
	D3DXMATRIX mProj;
};

