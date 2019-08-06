#pragma once

#include "D3DUtil.h"
#include "Vertex.h"
#include <vector>

class PSystem
{
public:
	PSystem(
		const std::string& fxName,
		const std::string& techName,
		const std::string& texName,
		const D3DXVECTOR3& accel,
		const AABB& box,
		int maxNumParticles,
		float timePerParticle);

	virtual ~PSystem();

	// Access Methods
	float GetTime();
	void  SetTime(float t);
	const AABB& GetAABB()const;

	void SetWorldMtx(const D3DXMATRIX& world);
	void AddParticle();

	virtual void OnLostDevice();
	virtual void OnResetDevice();

	virtual void InitParticle(Particle& out) = 0;
	virtual void Update(float dt);
	virtual void Draw();

protected:
	// In practice, some sort of ID3DXEffect and IDirect3DTexture9 manager should
	// be used so that you do not duplicate effects/textures by having several
	// instances of a particle system.
	ID3DXEffect* mFX;
	D3DXHANDLE mhTech;
	D3DXHANDLE mhWVP;
	D3DXHANDLE mhEyePosL;
	D3DXHANDLE mhTex;
	D3DXHANDLE mhTime;
	D3DXHANDLE mhAccel;
	D3DXHANDLE mhViewportHeight;

	IDirect3DTexture9* mTex;
	IDirect3DVertexBuffer9* mVB;
	D3DXMATRIX mWorld;
	D3DXMATRIX mInvWorld;
	float mTime;
	D3DXVECTOR3 mAccel;
	AABB mBox;
	int mMaxNumParticles;
	float mTimePerParticle;

	std::vector<Particle> mParticles;
	std::vector<Particle*> mAliveParticles;
	std::vector<Particle*> mDeadParticles;
};

