#pragma once
#include "GfxStats.h"
#include "D3DApp.h"

enum SolarType
{
	SUN,
	PLANET,
	MOON
};

struct SolarObject
{
	void Set(SolarType type, D3DXVECTOR3 p, float yRot, int parentIndex, float s, IDirect3DTexture9* t)
	{
		typeID = type;
		pos = p;
		yAngle = yRot;
		parent = parentIndex;
		size = s;
		tex = t;
	}

	// Note: The root's "parent" frame is the world space.

	SolarType typeID;
	D3DXVECTOR3 pos;  // Relative to parent frame.
	float yAngle;     // Relative to parent frame.
	int parent;       // Index to parent frame (-1 if root, i.e., no parent)
	float size;       // Relative to world frame.
	IDirect3DTexture9* tex;
	D3DXMATRIX toParentXForm;
	D3DXMATRIX toWorldXForm;
};

class SolarSysDemoApp : public D3DApp
{
public:
	SolarSysDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~SolarSysDemoApp();

	bool CheckDeviceCaps();
	void OnLostDevice();
	void OnResetDevice();
	void UpdateScene(float dt);
	void DrawScene();

	// Helper methods
	void BuildFX();
	void BuildViewMtx();
	void BuildProjMtx();

	void BuildObjectWorldTransforms();

	void GenSphericalTexCoords();

private:
	GfxStats* mGfxStats;

	// We only need one sphere mesh.  To draw several solar objects we just 
	// draw the same mesh several times, but with a different transformation
	// applied so that it is drawn in a different place.
	ID3DXMesh* mSphere;

	static const int NUM_OBJECTS = 10;
	SolarObject mObject[NUM_OBJECTS];

	IDirect3DTexture9* mSunTex;
	IDirect3DTexture9* mPlanet1Tex;
	IDirect3DTexture9* mPlanet2Tex;
	IDirect3DTexture9* mPlanet3Tex;
	IDirect3DTexture9* mMoonTex;

	// Use a white material--the color will come from the texture.
	Mtrl mWhiteMtrl;

	DirLight mLight;

	ID3DXEffect* mFX;
	D3DXHANDLE   mhTech;
	D3DXHANDLE   mhWVP;
	D3DXHANDLE   mhWorldInvTrans;
	D3DXHANDLE   mhEyePos;
	D3DXHANDLE   mhWorld;
	D3DXHANDLE   mhTex;
	D3DXHANDLE   mhMtrl;
	D3DXHANDLE   mhLight;

	float mCameraRotationY;
	float mCameraRadius;
	float mCameraHeight;

	D3DXMATRIX mWorld;
	D3DXMATRIX mView;
	D3DXMATRIX mProj;
};

