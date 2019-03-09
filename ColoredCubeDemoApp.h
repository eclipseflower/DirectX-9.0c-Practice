#pragma once
#include "GfxStats.h"
#include "D3DApp.h"

const D3DCOLOR WHITE = D3DCOLOR_XRGB(255, 255, 255);
const D3DCOLOR BLACK = D3DCOLOR_XRGB(0, 0, 0);
const D3DCOLOR RED = D3DCOLOR_XRGB(255, 0, 0);
const D3DCOLOR GREEN = D3DCOLOR_XRGB(0, 255, 0);
const D3DCOLOR BLUE = D3DCOLOR_XRGB(0, 0, 255);
const D3DCOLOR YELLOW = D3DCOLOR_XRGB(255, 255, 0);
const D3DCOLOR CYAN = D3DCOLOR_XRGB(0, 255, 255);
const D3DCOLOR MAGENTA = D3DCOLOR_XRGB(255, 0, 255);

class ColoredCubeDemoApp : public D3DApp
{
public:
	ColoredCubeDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~ColoredCubeDemoApp();

	bool CheckDeviceCaps();
	void OnLostDevice();
	void OnResetDevice();
	void UpdateScene(float dt);
	void DrawScene();

	void BuildVertexBuffer();
	void BuildIndexBuffer();
	void BuildFx();
	void BuildViewMtx();
	void BuildProjMtx();

private:
	GfxStats *mGfxStats;
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

