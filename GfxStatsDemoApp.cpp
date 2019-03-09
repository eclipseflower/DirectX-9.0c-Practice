#include "GfxStatsDemoApp.h"

GfxStatsDemoApp::GfxStatsDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP) : D3DApp(hInstance, winCaption, devType, requestedVP)
{
	if (!CheckDeviceCaps())
	{
		MessageBox(0, "checkDeviceCaps() Failed", 0, 0);
		PostQuitMessage(0);
	}

	mGfxStats = new GfxStats();
}

GfxStatsDemoApp::~GfxStatsDemoApp()
{
	delete mGfxStats;
}

bool GfxStatsDemoApp::CheckDeviceCaps()
{
	return true;
}

void GfxStatsDemoApp::OnLostDevice()
{
	mGfxStats->OnLostDevice();
}

void GfxStatsDemoApp::OnResetDevice()
{
	mGfxStats->OnResetDevice();
}

void GfxStatsDemoApp::UpdateScene(float dt)
{
	mGfxStats->Update(dt);
}

void GfxStatsDemoApp::DrawScene()
{
	HR(gD3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0));
	HR(gD3dDevice->BeginScene());
	mGfxStats->Display();
	HR(gD3dDevice->EndScene());
	HR(gD3dDevice->Present(0, 0, 0, 0));
}
