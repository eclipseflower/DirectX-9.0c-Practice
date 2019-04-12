#pragma once
#include "GfxStats.h"
#include "D3DApp.h"
class GateDemoApp : public D3DApp
{
public:
	GateDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~GateDemoApp();

	bool CheckDeviceCaps();
	void OnLostDevice();
	void OnResetDevice();
	void UpdateScene(float dt);
	void DrawScene();

	// Helper methods
	void BuildGridGeometry();
	void BuildGateGeometry();
	void BuildFX();
	void BuildViewMtx();
	void BuildProjMtx();

	void DrawGround();
	void DrawGate();

private:
	GfxStats* mGfxStats;

	DWORD mNumGridVertices;
	DWORD mNumGridTriangles;

	IDirect3DVertexBuffer9* mGridVB;
	IDirect3DIndexBuffer9*  mGridIB;
	IDirect3DVertexBuffer9* mGateVB;
	IDirect3DIndexBuffer9*  mGateIB;
	IDirect3DTexture9*     mGroundTex;
	IDirect3DTexture9*     mGateTex;

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
	D3DXHANDLE   mhTex;

	Mtrl mGroundMtrl;
	Mtrl mGateMtrl;

	D3DXVECTOR3 mLightVecW;
	D3DXCOLOR   mAmbientLight;
	D3DXCOLOR   mDiffuseLight;
	D3DXCOLOR   mSpecularLight;

	float mCameraRotationY;
	float mCameraRadius;
	float mCameraHeight;

	D3DXMATRIX mGroundWorld;
	D3DXMATRIX mGateWorld;
	D3DXMATRIX mView;
	D3DXMATRIX mProj;
};

