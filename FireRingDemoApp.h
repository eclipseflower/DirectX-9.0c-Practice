#pragma once
#include "PSystem.h"
#include "D3DApp.h"
#include "GfxStats.h"
#include "Terrain.h"

class FireRing : public PSystem
{
public:
	FireRing(const std::string& fxName,
		const std::string& techName,
		const std::string& texName,
		const D3DXVECTOR3& accel,
		const AABB& box,
		int maxNumParticles,
		float timePerParticle)
		: PSystem(fxName, techName, texName, accel, box,
			maxNumParticles, timePerParticle)
	{
	}

	void InitParticle(Particle& out)
	{
		// Time particle is created relative to the global running
		// time of the particle system.
		out.initialTime = mTime;

		// Flare lives for 2-4 seconds.
		out.lifeTime = GetRandomFloat(2.0f, 4.0f);

		// Initial size in pixels.
		out.initialSize = GetRandomFloat(10.0f, 15.0f);

		// Give a very small initial velocity to give the flares
		// some randomness.
		GetRandomVec(out.initialVelocity);

		// Scalar value used in vertex shader as an amplitude factor.
		out.mass = GetRandomFloat(1.0f, 2.0f);

		// Start color at 50-100% intensity when born for variation.
		out.initialColor = GetRandomFloat(0.5f, 1.0f) * _WHITE;

		// Generate random particle on the ring in polar coordinates:
		// random radius and random angle.
		float r = GetRandomFloat(10.0f, 14.0f);
		float t = GetRandomFloat(0, 2.0f*D3DX_PI);

		// Convert to Cartesian coordinates.
		out.initialPos.x = r * cosf(t);
		out.initialPos.y = r * sinf(t);

		// Random depth value in [-1, 1] (depth of the ring)
		out.initialPos.z = GetRandomFloat(-1.0f, 1.0f);
	}
};

class FireRingDemoApp : public D3DApp
{
public:
	FireRingDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~FireRingDemoApp();

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