#pragma once
#include "PSystem.h"
#include "Camera.h"
#include "D3DApp.h"
#include "GfxStats.h"

class Rain : public PSystem
{
public:
	Rain(const std::string& fxName,
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
		// Generate about the camera.
		out.initialPos = gCamera->Pos();

		// Spread the particles out on xz-plane.
		out.initialPos.x += GetRandomFloat(-100.0f, 100.0f);
		out.initialPos.z += GetRandomFloat(-100.0f, 100.0f);

		// Generate above the camera.
		out.initialPos.y += GetRandomFloat(50.0f, 55.0f);

		out.initialTime = mTime;
		out.lifeTime = GetRandomFloat(2.0f, 2.5f);
		out.initialColor = _WHITE;
		out.initialSize = GetRandomFloat(6.0f, 7.0f);

		// Give them an initial falling down velocity.
		out.initialVelocity.x = GetRandomFloat(-1.5f, 0.0f);
		out.initialVelocity.y = GetRandomFloat(-50.0f, -45.0f);
		out.initialVelocity.z = GetRandomFloat(-0.5f, 0.5f);
	}
};

class RainDemoApp : public D3DApp
{
public:
	RainDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~RainDemoApp();

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

