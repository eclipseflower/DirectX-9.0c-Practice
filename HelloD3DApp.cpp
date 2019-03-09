#include "HelloD3DApp.h"

HelloD3DApp::HelloD3DApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP) : D3DApp(hInstance, winCaption, devType, requestedVP)
{
	srand(time_t(0));

	if (!CheckDeviceCaps())
	{
		MessageBox(0, "checkDeviceCaps() Failed", 0, 0);
		PostQuitMessage(0);
	}

	D3DXFONT_DESC fontDesc;
	fontDesc.Height = 80;
	fontDesc.Width = 40;
	fontDesc.Weight = FW_BOLD;
	fontDesc.MipLevels = 0;
	fontDesc.Italic = true;
	fontDesc.CharSet = DEFAULT_CHARSET;
	fontDesc.OutputPrecision = OUT_DEFAULT_PRECIS;
	fontDesc.Quality = DEFAULT_QUALITY;
	fontDesc.PitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	strcpy_s(fontDesc.FaceName, _T("Times New Roman"));

	HR(D3DXCreateFontIndirect(gD3dDevice, &fontDesc, &mFont));
}

HelloD3DApp::~HelloD3DApp()
{
	ReleaseCOM(mFont);
}

bool HelloD3DApp::CheckDeviceCaps()
{
	return true;
}

void HelloD3DApp::OnLostDevice()
{
	HR(mFont->OnLostDevice());
}

void HelloD3DApp::OnResetDevice()
{
	HR(mFont->OnResetDevice());
}

void HelloD3DApp::UpdateScene(float dt)
{
}

void HelloD3DApp::DrawScene()
{
	HR(gD3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(255, 255, 255), 1.0f, 0));
	RECT formatRect;
	GetClientRect(mMainWnd, &formatRect);
	HR(gD3dDevice->BeginScene());
	mFont->DrawText(0, _T("Hello Direct3D"), -1, &formatRect, DT_CENTER | DT_VCENTER, D3DCOLOR_XRGB(rand() % 256, rand() % 256, rand() % 256));
	HR(gD3dDevice->EndScene());
	HR(gD3dDevice->Present(0, 0, 0, 0));
}