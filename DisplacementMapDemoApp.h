#pragma once
#include "D3DApp.h"
#include "GfxStats.h"
#include "WaterDMap.h"
#include "Sky.h"

class DisplacementMapDemoApp : public D3DApp
{
public:
	DisplacementMapDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~DisplacementMapDemoApp();

	bool CheckDeviceCaps();
	void OnLostDevice();
	void OnResetDevice();
	void UpdateScene(float dt);
	void DrawScene();

private:
	GfxStats* mGfxStats;
	Sky* mSky;
	WaterDMap* mWater;
	DirLight mLight;
};

