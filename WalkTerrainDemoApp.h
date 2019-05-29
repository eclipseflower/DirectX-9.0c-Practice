#pragma once
#include "GfxStats.h"
#include "Terrain.h"
#include "D3DApp.h"
class WalkTerrainDemoApp : public D3DApp
{
public:
	WalkTerrainDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~WalkTerrainDemoApp();

	bool CheckDeviceCaps();
	void OnLostDevice();
	void OnResetDevice();
	void UpdateScene(float dt);
	void DrawScene();

private:
	GfxStats* mGfxStats;
	Terrain*  mTerrain;
};

