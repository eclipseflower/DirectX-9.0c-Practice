#pragma once
#include "GfxStats.h"
#include "D3DApp.h"
class SphereCylDemoApp : public D3DApp
{
public:
	SphereCylDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~SphereCylDemoApp();

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

	enum AXIS
	{
		X_AXIS,
		Y_AXIS,
		Z_AXIS
	};

	void GenSphericalTexCoords();
	void GenCylTexCoords(AXIS axis);

private:
	GfxStats* mGfxStats;

	DWORD mNumGridVertices;
	DWORD mNumGridTriangles;

	ID3DXMesh* mCylinder;
	ID3DXMesh* mSphere;

	IDirect3DVertexBuffer9* mVB;
	IDirect3DIndexBuffer9*  mIB;

	IDirect3DTexture9* mSphereTex;
	IDirect3DTexture9* mCylTex;
	IDirect3DTexture9* mGridTex;

	ID3DXEffect* mFX;
	D3DXHANDLE   mhTech;
	D3DXHANDLE   mhWVP;
	D3DXHANDLE   mhWorldInvTrans;
	D3DXHANDLE   mhAmbientLight;
	D3DXHANDLE   mhDiffuseLight;
	D3DXHANDLE   mhSpecLight;
	D3DXHANDLE   mhLightVecW;
	D3DXHANDLE   mhAmbientMtrl;
	D3DXHANDLE   mhDiffuseMtrl;
	D3DXHANDLE   mhSpecMtrl;
	D3DXHANDLE   mhSpecPower;
	D3DXHANDLE   mhEyePos;
	D3DXHANDLE   mhWorld;
	D3DXHANDLE   mhTex;

	D3DXCOLOR   mAmbientLight;
	D3DXCOLOR   mDiffuseLight;
	D3DXCOLOR   mSpecLight;
	D3DXVECTOR3 mLightVecW;

	Mtrl  mGridMtrl;
	Mtrl  mCylinderMtrl;
	Mtrl  mSphereMtrl;

	float mCameraRotationY;
	float mCameraRadius;
	float mCameraHeight;

	D3DXMATRIX mView;
	D3DXMATRIX mProj;
};

