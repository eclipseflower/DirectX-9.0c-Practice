#pragma once
#include "D3DApp.h"
#include "GfxStats.h"

class GfxStatsDemoApp : public D3DApp
{
public:
	GfxStatsDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~GfxStatsDemoApp();

	bool CheckDeviceCaps();
	void OnLostDevice();
	void OnResetDevice();
	void UpdateScene(float dt);
	void DrawScene();

private:
	GfxStats* mGfxStats;
};

