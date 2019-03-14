#pragma once
#include "GfxStats.h"
#include "D3DApp.h"

class AmbientDiffuseSpecularDemoApp : public D3DApp
{
public:
	AmbientDiffuseSpecularDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~AmbientDiffuseSpecularDemoApp();

	bool CheckDeviceCaps();
	void OnLostDevice();
	void OnResetDevice();
	void UpdateScene(float dt);
	void DrawScene();

	void BuildFx();
	void BuildViewMtx();
	void BuildProjMtx();

private:
	GfxStats* mGfxStats;

	ID3DXMesh*   mTeapot;
	ID3DXEffect* mFx;
	D3DXHANDLE   mhTech;
	D3DXHANDLE   mhWVP;
	D3DXHANDLE   mhWorldInverseTranspose;
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

