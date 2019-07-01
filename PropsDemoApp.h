#pragma once
#include "D3DApp.h"
#include "Vertex.h"
#include "GfxStats.h"
#include "Terrain.h"
#include "Water.h"
#include "Camera.h"

struct Object3D
{
	Object3D()
	{
		mesh = 0;
	}
	~Object3D()
	{
		ReleaseCOM(mesh);
		for (UINT i = 0; i < textures.size(); ++i)
			ReleaseCOM(textures[i]);
	}

	ID3DXMesh* mesh;
	std::vector<Mtrl> mtrls;
	std::vector<IDirect3DTexture9*> textures;
	AABB box;
};

class PropsDemoApp : public D3DApp
{
public:
	PropsDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~PropsDemoApp();

	bool CheckDeviceCaps();
	void OnLostDevice();
	void OnResetDevice();
	void UpdateScene(float dt);
	void DrawScene();

	void BuildFX();
	void DrawObject(Object3D& obj, const D3DXMATRIX& toWorld);

	void BuildCastle();
	void BuildTrees();
	void BuildGrass();
	void BuildGrassFin(GrassVertex* v, WORD* k, int& indexOffset,
		D3DXVECTOR3& worldPos, D3DXVECTOR3& scale);

private:
	GfxStats* mGfxStats;
	Terrain*  mTerrain;
	Water*    mWater;

	float mTime; // Time elapsed from program start.

	// Models
	Object3D mCastle;
	D3DXMATRIX mCastleWorld;
	Object3D mTrees[4];
	static const int NUM_TREES = 200;
	D3DXMATRIX mTreeWorlds[NUM_TREES];

	static const int NUM_GRASS_BLOCKS = 4000;
	ID3DXMesh* mGrassMesh;
	IDirect3DTexture9* mGrassTex;

	// Grass FX
	ID3DXEffect* mGrassFX;
	D3DXHANDLE mhGrassTech;
	D3DXHANDLE mhGrassViewProj;
	D3DXHANDLE mhGrassTex;
	D3DXHANDLE mhGrassTime;
	D3DXHANDLE mhGrassEyePosW;
	D3DXHANDLE mhGrassDirToSunW;

	// General light/texture FX
	ID3DXEffect* mFX;
	D3DXHANDLE   mhTech;
	D3DXHANDLE   mhWVP;
	D3DXHANDLE   mhWorldInvTrans;
	D3DXHANDLE   mhEyePosW;
	D3DXHANDLE   mhWorld;
	D3DXHANDLE   mhTex;
	D3DXHANDLE   mhMtrl;
	D3DXHANDLE   mhLight;

	// The sun.
	DirLight mLight;

	// Camera fixed to ground or can fly?
	bool mFreeCamera;

	// Default texture if no texture present for subset.
	IDirect3DTexture9* mWhiteTex;
};

