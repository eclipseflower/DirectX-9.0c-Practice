#pragma once
#include "D3DApp.h"
#include "PSystem.h"
#include "Camera.h"
#include "GfxStats.h"

class Gun : public PSystem
{
public:
	Gun(const std::string& fxName,
		const std::string& techName,
		const std::string& texName,
		const D3DXVECTOR3& accel,
		const AABB& box,
		int maxNumParticles,
		float timePerParticle)
		: PSystem(fxName, techName, texName, accel, box, maxNumParticles,
			timePerParticle)
	{
	}

	void InitParticle(Particle& out)
	{
		// Generate at camera.
		out.initialPos = gCamera->Pos();

		// Set down a bit so it looks like player is carrying the gun.
		out.initialPos.y -= 3.0f;

		// Fire in camera's look direction.
		float speed = 500.0f;
		out.initialVelocity = speed * gCamera->Look();

		out.initialTime = mTime;
		out.lifeTime = 4.0f;
		out.initialColor = _WHITE;
		out.initialSize = GetRandomFloat(80.0f, 90.0f);
		out.mass = 1.0f;
	}
};

class GunDemoApp : public D3DApp
{
public:
	GunDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~GunDemoApp();

	bool CheckDeviceCaps();
	void OnLostDevice();
	void OnResetDevice();
	void UpdateScene(float dt);
	void DrawScene();

private:
	GfxStats* mGfxStats;
	Terrain*  mTerrain;
	PSystem*  mPSys;
};

