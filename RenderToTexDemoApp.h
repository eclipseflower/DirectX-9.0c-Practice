#pragma once
#include "D3DApp.h"
#include "GfxStats.h"
#include "Terrain.h"
#include "Sky.h"
#include "Camera.h"
#include "DrawableTex2D.h"

class RenderToTexDemoApp : public D3DApp
{
public:
	RenderToTexDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~RenderToTexDemoApp();

	bool CheckDeviceCaps();
	void OnLostDevice();
	void OnResetDevice();
	void UpdateScene(float dt);
	void DrawScene();

	void BuildFX();

private:
	GfxStats* mGfxStats;
	Terrain*  mTerrain;
	Sky* mSky;

	// Two camera's for this demo.
	Camera mFirstPersonCamera;
	Camera mBirdsEyeCamera;

	// The texture we draw into.
	DrawableTex2D* mRadarMap;
	bool mAutoGenMips;

	IDirect3DVertexBuffer9* mRadarVB;

	// General light/texture FX
	ID3DXEffect* mRadarFX;
	D3DXHANDLE   mhTech;
	D3DXHANDLE   mhTex;
};

