#pragma once

#include <d3dx9.h>
#include "D3DUtil.h"

class Terrain;

class Camera
{
public:
	Camera();

	const D3DXMATRIX& View() const;
	const D3DXMATRIX& Proj() const;
	const D3DXMATRIX& ViewProj() const;

	const D3DXVECTOR3& Right() const;
	const D3DXVECTOR3& Up() const;
	const D3DXVECTOR3& Look() const;

	D3DXVECTOR3& Pos();

	void LookAt(D3DXVECTOR3& pos, D3DXVECTOR3& target, D3DXVECTOR3& up);
	void SetLens(float fov, float aspect, float nearZ, float farZ);
	void SetSpeed(float s);

	void Update(float dt, Terrain* terrain, float offsetHeight);

	// Box coordinates should be relative to world space.
	bool IsVisible(const AABB& box) const;

protected:
	void BuildView();
	void BuildWorldFrustumPlanes();

protected:
	D3DXMATRIX mView;
	D3DXMATRIX mProj;
	D3DXMATRIX mViewProj;

	// Relative to world space.
	D3DXVECTOR3 mPosW;
	D3DXVECTOR3 mRightW;
	D3DXVECTOR3 mUpW;
	D3DXVECTOR3 mLookW;

	float mSpeed;

	// Frustum Planes
	D3DXPLANE mFrustumPlanes[6]; // [0] = near
								 // [1] = far
								 // [2] = left
								 // [3] = right
								 // [4] = top
								 // [5] = bottom
};