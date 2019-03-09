#pragma once

#include "D3DUtil.h"
#include <string>

class D3DApp
{
public:
	D3DApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	virtual ~D3DApp();

	HINSTANCE GetAppInst();
	HWND      GetMainWnd();

	virtual bool CheckDeviceCaps() { return true; }
	virtual void OnLostDevice() {}
	virtual void OnResetDevice() {}
	virtual void UpdateScene(float dt) {}
	virtual void DrawScene() {}

	virtual void InitMainWindow();
	virtual void InitDirect3D();
	virtual int Run();
	virtual LRESULT MsgProc(UINT msg, WPARAM wParam, LPARAM lParam);

	void EnableFullScreenMode(bool enable);
	bool IsDeviceLost();

protected: 
	std::string mMainWndCaption;
	D3DDEVTYPE  mDevType;
	DWORD       mRequestedVP;

	HINSTANCE             mAppInst;
	HWND                  mMainWnd;
	IDirect3D9*           mD3dObject;
	bool                  mAppPaused;
	D3DPRESENT_PARAMETERS mD3dpp;
};

extern D3DApp* gD3dApp;
extern IDirect3DDevice9* gD3dDevice;