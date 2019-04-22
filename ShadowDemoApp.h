#pragma once
#include "GfxStats.h"
#include "D3DApp.h"
class ShadowDemoApp : public D3DApp
{
public:
	ShadowDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~ShadowDemoApp();

	bool CheckDeviceCaps();
	void OnLostDevice();
	void OnResetDevice();
	void UpdateScene(float dt);
	void DrawScene();

	// Helper methods
	void BuildRoomGeometry();
	void BuildFX();
	void BuildViewMtx();
	void BuildProjMtx();

	void DrawRoom();
	void DrawMirror();
	void DrawTeapot();
	void DrawReflectedTeapot();
	void DrawTeapotShadow();
	void GenSphericalTexCoords();

private:
	GfxStats* mGfxStats;

	IDirect3DVertexBuffer9* mRoomVB;
	ID3DXMesh*              mTeapot;

	IDirect3DTexture9*     mFloorTex;
	IDirect3DTexture9*     mWallTex;
	IDirect3DTexture9*     mMirrorTex;
	IDirect3DTexture9*     mTeapotTex;

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

	Mtrl mWhiteMtrl;
	Mtrl mShadowMtrl;

	D3DXVECTOR3 mLightVecW;
	D3DXCOLOR   mAmbientLight;
	D3DXCOLOR   mDiffuseLight;
	D3DXCOLOR   mSpecularLight;

	float mCameraRotationY;
	float mCameraRadius;
	float mCameraHeight;

	D3DXMATRIX mRoomWorld;
	D3DXMATRIX mTeapotWorld;

	D3DXMATRIX mView;
	D3DXMATRIX mProj;
};

