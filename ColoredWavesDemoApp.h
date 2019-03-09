#pragma once
#include "GfxStats.h"
#include "D3DApp.h"
class ColoredWavesDemoApp : public D3DApp
{
public:
	ColoredWavesDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~ColoredWavesDemoApp();

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
private:
	GfxStats* mGfxStats;

	DWORD mNumVertices;
	DWORD mNumTriangles;

	IDirect3DVertexBuffer9* mVb;
	IDirect3DIndexBuffer9*  mIb;
	ID3DXEffect*            mFx;
	D3DXHANDLE              mhTech;
	D3DXHANDLE              mhWVP;
	D3DXHANDLE              mhTime;

	float mTime;

	float mCameraRotationY;
	float mCameraRadius;
	float mCameraHeight;

	D3DXMATRIX mView;
	D3DXMATRIX mProj;
};

