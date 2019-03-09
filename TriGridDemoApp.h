#pragma once
#include "GfxStats.h"
#include "D3DApp.h"

class TriGridDemoApp : public D3DApp
{
public:
	TriGridDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~TriGridDemoApp();

	bool CheckDeviceCaps();
	void OnLostDevice();
	void OnResetDevice();
	void UpdateScene(float dt);
	void DrawScene();

	void BuildGeoBuffers();
	void BuildFx();
	void BuildViewMtx();
	void BuildProjMtx();

private:
	GfxStats *mGfxStats;
	DWORD mNumVertices;
	DWORD mNumTriangles;
	IDirect3DVertexBuffer9 *mVb;
	IDirect3DIndexBuffer9 *mIb;
	ID3DXEffect *mFx;
	D3DXHANDLE mhTech;
	D3DXHANDLE mhWVP;
	float mCameraRotationY;
	float mCameraRadius;
	float mCameraHeight;
	D3DXMATRIX mView;
	D3DXMATRIX mProj;
};

