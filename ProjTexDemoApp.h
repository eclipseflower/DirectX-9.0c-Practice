#pragma once
#include "D3DApp.h"
#include "GfxStats.h"
#include "Sky.h"

struct SpotLight
{
	D3DXCOLOR ambient;
	D3DXCOLOR diffuse;
	D3DXCOLOR spec;
	D3DXVECTOR3 posW;
	D3DXVECTOR3 dirW;
	float  spotPower;
};

class ProjTexDemoApp : public D3DApp
{
public:
	ProjTexDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~ProjTexDemoApp();

	bool CheckDeviceCaps();
	void OnLostDevice();
	void OnResetDevice();
	void UpdateScene(float dt);
	void DrawScene();

	void BuildFX();
private:
	GfxStats* mGfxStats;

	Sky* mSky;
	ID3DXMesh* mSceneMesh;
	D3DXMATRIX mSceneWorld;
	std::vector<Mtrl> mSceneMtrls;
	std::vector<IDirect3DTexture9*> mSceneTextures;

	IDirect3DTexture9* mWhiteTex;
	IDirect3DTexture9* mSkullTex;

	D3DXMATRIX mLightWVP;

	ID3DXEffect* mFX;
	D3DXHANDLE   mhTech;
	D3DXHANDLE   mhWVP;
	D3DXHANDLE   mhLightWVP;
	D3DXHANDLE   mhWorldInvTrans;
	D3DXHANDLE   mhEyePosW;
	D3DXHANDLE   mhWorld;
	D3DXHANDLE   mhTex;
	D3DXHANDLE   mhMtrl;
	D3DXHANDLE   mhLight;

	SpotLight mLight;
};

