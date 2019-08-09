#pragma once
#include "D3DApp.h"
#include "PSystem.h"
#include "GfxStats.h"
#include "Terrain.h"

class Sprinkler : public PSystem
{
public:
	Sprinkler(const std::string& fxName,
		const std::string& techName,
		const std::string& texName,
		const D3DXVECTOR3& accel,
		const AABB& box,
		int maxNumParticles,
		float timePerSecond)
		: PSystem(fxName, techName, texName, accel, box, maxNumParticles,
			timePerSecond)
	{
	}

	void InitParticle(Particle& out)
	{
		// Generate about the origin.
		out.initialPos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

		out.initialTime = mTime;
		out.lifeTime = GetRandomFloat(4.0f, 5.0f);
		out.initialColor = _WHITE;
		out.initialSize = GetRandomFloat(8.0f, 12.0f);
		out.mass = GetRandomFloat(0.8f, 1.2f);

		out.initialVelocity.x = GetRandomFloat(-2.5f, 2.5f);
		out.initialVelocity.y = GetRandomFloat(15.0f, 25.0f);
		out.initialVelocity.z = GetRandomFloat(-2.5f, 2.5f);
	}
};

class SprinklerDemoApp :
	public D3DApp
{
public:
	SprinklerDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~SprinklerDemoApp();

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

