#pragma once

#include <d3dx9.h>

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

protected:
	void BuildView();

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
};