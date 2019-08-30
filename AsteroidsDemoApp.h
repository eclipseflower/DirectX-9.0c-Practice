#pragma once
#include "D3DApp.h"
#include "PSystem.h"
#include "GfxStats.h"
#include <list>

class FireWork : public PSystem
{
public:
	FireWork(const std::string& fxName,
		const std::string& techName,
		const std::string& texName,
		const D3DXVECTOR3& accel,
		const AABB& box,
		int maxNumParticles,
		float timePerParticle)
		: PSystem(fxName, techName, texName, accel, box,
			maxNumParticles, timePerParticle)
	{
		for (int i = 0; i < mMaxNumParticles; ++i)
		{
			InitParticle(mParticles[i]);
		}
	}

	void InitParticle(Particle& out)
	{
		out.initialTime = 0.0f;
		out.initialSize = GetRandomFloat(12.0f, 15.0f);
		out.lifeTime = 10.0f;

		// Generate Random Direction
		D3DXVECTOR3 d;
		GetRandomVec(d);

		// Compute velocity.
		float speed = GetRandomFloat(100.0f, 150.0f);
		out.initialVelocity = speed * d;

		out.initialColor = _WHITE;
		out.mass = GetRandomFloat(2.0f, 4.0f);

		float r = GetRandomFloat(0.0f, 2.0f);
		out.initialPos = r * d;
	}
};

struct FireWorkInstance
{
	float time;
	D3DXMATRIX toWorld;
};

struct Asteroid
{
	D3DXVECTOR3 axis;
	float theta;
	D3DXVECTOR3 pos;
	D3DXVECTOR3 vel;
};

class AsteroidsDemoApp : public D3DApp
{
public:
	AsteroidsDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~AsteroidsDemoApp();

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

	// We only need one firework system, as we just draw the same system
	// several times per frame in different positions and at different
	// relative times to simulate multiple systems.
	PSystem* mFireWork;

	// A list of firework *instances*
	std::list<FireWorkInstance> mFireWorkInstances;

	// A list of asteroids.
	static const int NUM_ASTEROIDS = 300;
	std::list<Asteroid> mAsteroids;

	// We only need one actual mesh, as we just draw the same mesh several
	// times per frame in different positions to simulate multiple asteroids.
	ID3DXMesh* mAsteroidMesh;
	std::vector<Mtrl> mAsteroidMtrls;
	std::vector<IDirect3DTexture9*> mAsteroidTextures;
	AABB mAsteroidBox;

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

