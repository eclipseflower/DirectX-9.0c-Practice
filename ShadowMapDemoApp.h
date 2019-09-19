#pragma once
#include "D3DApp.h"
#include "GfxStats.h"
#include "Sky.h"
#include "DrawableTex2D.h"

struct SpotLight
{
	D3DXCOLOR ambient;
	D3DXCOLOR diffuse;
	D3DXCOLOR spec;
	D3DXVECTOR3 posW;
	D3DXVECTOR3 dirW;
	float  spotPower;
};

class ShadowMapDemoApp : public D3DApp
{
public:
	ShadowMapDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~ShadowMapDemoApp();

	bool CheckDeviceCaps();
	void OnLostDevice();
	void OnResetDevice();
	void UpdateScene(float dt);
	void DrawScene();
	void DrawShadowMap();
	void BuildFX();
private:
	GfxStats* mGfxStats;

	Sky* mSky;
	ID3DXMesh* mSceneMesh;
	D3DXMATRIX mSceneWorld;
	std::vector<Mtrl> mSceneMtrls;
	std::vector<IDirect3DTexture9*> mSceneTextures;

	ID3DXMesh* mCarMesh;
	D3DXMATRIX mCarWorld;
	std::vector<Mtrl> mCarMtrls;
	std::vector<IDirect3DTexture9*> mCarTextures;

	IDirect3DTexture9* mWhiteTex;
	DrawableTex2D* mShadowMap;

	D3DXMATRIX mLightVP;

	ID3DXEffect* mFX;
	D3DXHANDLE   mhBuildShadowMapTech;
	D3DXHANDLE   mhLightWVP;

	D3DXHANDLE   mhTech;
	D3DXHANDLE   mhWVP;
	D3DXHANDLE   mhWorldInvTrans;
	D3DXHANDLE   mhEyePosW;
	D3DXHANDLE   mhWorld;
	D3DXHANDLE   mhTex;
	D3DXHANDLE   mhShadowMap;
	D3DXHANDLE   mhMtrl;
	D3DXHANDLE   mhLight;

	SpotLight mLight;
};

