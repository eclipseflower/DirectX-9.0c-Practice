#pragma once

#include "D3DApp.h"
#include <tchar.h>
#include <crtdbg.h>

class HelloD3DApp : public D3DApp
{
public:
	HelloD3DApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~HelloD3DApp();

	bool CheckDeviceCaps();
	void OnLostDevice();
	void OnResetDevice();
	void UpdateScene(float dt);
	void DrawScene();

private:
	ID3DXFont* mFont;
};