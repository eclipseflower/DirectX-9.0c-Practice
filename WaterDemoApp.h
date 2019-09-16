#pragma once
#include "D3DApp.h"
#include "GfxStats.h"
#include "Sky.h"
#include "Water.h"

class WaterDemoApp : public D3DApp
{
public:
	WaterDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~WaterDemoApp();

	bool CheckDeviceCaps();
	void OnLostDevice();
	void OnResetDevice();
	void UpdateScene(float dt);
	void DrawScene();

	void BuildFX();
private:
	GfxStats* mGfxStats;

	Water* mWater;
	Sky* mSky;
	ID3DXMesh* mSceneMesh;
	D3DXMATRIX mSceneWorld;
	D3DXMATRIX mSceneWorldInv;
	std::vector<Mtrl> mSceneMtrls;
	std::vector<IDirect3DTexture9*> mSceneTextures;

	// Hack for this particular scene--usually you'd want to come up
	// with a more general method of loading normal maps such that
	// the ith normal map corresponds with the ith mesh subset.
	// For example, you might call each color texture name_color and 
	// its corresponding normal map name_nmap.  Then when you load the
	// name_color texture you also load the corresponding normal map.
	// If a texture doesn't have a normal map, you could use a default one
	// like we use the default white texture.
	IDirect3DTexture9* mSceneNormalMaps[2];

	IDirect3DTexture9* mWhiteTex;

	ID3DXEffect* mFX;
	D3DXHANDLE   mhTech;
	D3DXHANDLE   mhWVP;
	D3DXHANDLE   mhWorldInv;
	D3DXHANDLE   mhEyePosW;
	D3DXHANDLE   mhTex;
	D3DXHANDLE   mhMtrl;
	D3DXHANDLE   mhLight;
	D3DXHANDLE   mhNormalMap;

	DirLight mLight;
};

