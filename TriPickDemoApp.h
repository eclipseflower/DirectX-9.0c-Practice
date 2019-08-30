#pragma once
#include "D3DApp.h"
#include "GfxStats.h"

class TriPickDemoApp : public D3DApp
{
public:
	TriPickDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~TriPickDemoApp();

	bool CheckDeviceCaps();
	void OnLostDevice();
	void OnResetDevice();
	void UpdateScene(float dt);
	void DrawScene();

	void InitAsteroids();
	void BuildFX();
	void GetWorldPickingRay(D3DXVECTOR3& originW, D3DXVECTOR3& dirW);

private:
	GfxStats* mGfxStats;

	// Car mesh.
	ID3DXMesh* mMesh;
	std::vector<Mtrl> mMeshMtrls;
	std::vector<IDirect3DTexture9*> mMeshTextures;

	// General light/texture FX
	ID3DXEffect* mFX;
	D3DXHANDLE   mhTech;
	D3DXHANDLE   mhWVP;
	D3DXHANDLE   mhWorldInvTrans;
	D3DXHANDLE   mhEyePos;
	D3DXHANDLE   mhWorld;
	D3DXHANDLE   mhTex;
	D3DXHANDLE   mhMtrl;
	D3DXHANDLE   mhLight;

	DirLight mLight;

	// Default texture if no texture present for subset.
	IDirect3DTexture9* mWhiteTex;
};

