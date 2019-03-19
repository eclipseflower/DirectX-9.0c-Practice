#pragma once
#include "GfxStats.h"
#include "D3DApp.h"
class SpotlightDemoApp : public D3DApp
{
public:
	SpotlightDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~SpotlightDemoApp();

	bool CheckDeviceCaps();
	void OnLostDevice();
	void OnResetDevice();
	void UpdateScene(float dt);
	void DrawScene();

	// Helper methods
	void BuildGeoBuffers();
	void BuildFx();
	void BuildViewMtx();
	void BuildProjMtx();

	void DrawGrid();
	void DrawCylinders();
	void DrawSpheres();

private:
	GfxStats* mGfxStats;

	DWORD mNumGridVertices;
	DWORD mNumGridTriangles;

	ID3DXMesh* mCylinder;
	ID3DXMesh* mSphere;

	IDirect3DVertexBuffer9* mVb;
	IDirect3DIndexBuffer9*  mIb;

	ID3DXEffect* mFx;
	D3DXHANDLE   mhTech;
	D3DXHANDLE   mhWVP;
	D3DXHANDLE   mhWorldInvTrans;
	D3DXHANDLE   mhAmbientLight;
	D3DXHANDLE   mhDiffuseLight;
	D3DXHANDLE   mhSpecLight;
	D3DXHANDLE   mhLightPosW;
	D3DXHANDLE   mhLightDirW;
	D3DXHANDLE   mhAttenuation012;
	D3DXHANDLE   mhSpotPower;
	D3DXHANDLE   mhAmbientMtrl;
	D3DXHANDLE   mhDiffuseMtrl;
	D3DXHANDLE   mhSpecMtrl;
	D3DXHANDLE   mhSpecPower;
	D3DXHANDLE   mhEyePos;
	D3DXHANDLE   mhWorld;

	D3DXCOLOR   mAmbientLight;
	D3DXCOLOR   mDiffuseLight;
	D3DXCOLOR   mSpecLight;
	D3DXVECTOR3 mAttenuation012;
	float       mSpotPower;

	Mtrl  mGridMtrl;
	Mtrl  mCylinderMtrl;
	Mtrl  mSphereMtrl;

	float mCameraRotationY;
	float mCameraRadius;
	float mCameraHeight;

	D3DXMATRIX mView;
	D3DXMATRIX mProj;
};

