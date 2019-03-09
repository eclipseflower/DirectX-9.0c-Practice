#pragma once
#include "D3DApp.h"
#include "GfxStats.h"

class CubeDemoApp : public D3DApp
{
public:
	CubeDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~CubeDemoApp();

	bool CheckDeviceCaps();
	void OnLostDevice();
	void OnResetDevice();
	void UpdateScene(float dt);
	void DrawScene();

	void BuildVertexBuffer();
	void BuildIndexBuffer();
	void BuildViewMtx();
	void BuildProjMtx();

private:
	GfxStats *mGfxStats;
	IDirect3DVertexBuffer9 *mVb;
	IDirect3DIndexBuffer9 *mIb;

	float mCameraRotationY;
	float mCameraRadius;
	float mCameraHeight;

	D3DXMATRIX mView;
	D3DXMATRIX mProj;
};

