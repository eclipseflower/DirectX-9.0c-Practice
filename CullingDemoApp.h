#pragma once
#include "D3DApp.h"
#include "GfxStats.h"
#include "Terrain.h"

class CullingDemoApp : public D3DApp
{
public:
	CullingDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~CullingDemoApp();

	bool CheckDeviceCaps();
	void OnLostDevice();
	void OnResetDevice();
	void UpdateScene(float dt);
	void DrawScene();

private:
	GfxStats* mGfxStats;
	Terrain*  mTerrain;
};

