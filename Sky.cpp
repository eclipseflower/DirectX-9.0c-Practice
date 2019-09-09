#include "Sky.h"
#include "d3dUtil.h"
#include "Camera.h"

Sky::Sky(const std::string & envmapFilename, float skyRadius) : mRadius(skyRadius)
{
	HR(D3DXCreateSphere(gD3dDevice, skyRadius, 30, 30, &mSphere, 0));
	HR(D3DXCreateCubeTextureFromFile(gD3dDevice, envmapFilename.c_str(), &mEnvMap));

	ID3DXBuffer* errors = 0;
	HR(D3DXCreateEffectFromFile(gD3dDevice, "sky.fx", 0, 0, 0,
		0, &mFX, &errors));
	if (errors)
		MessageBox(0, (char*)errors->GetBufferPointer(), 0, 0);

	mhTech = mFX->GetTechniqueByName("SkyTech");
	mhWVP = mFX->GetParameterByName(0, "gWVP");
	mhEnvMap = mFX->GetParameterByName(0, "gEnvMap");

	// Set effect parameters that do not vary.
	HR(mFX->SetTechnique(mhTech));
	HR(mFX->SetTexture(mhEnvMap, mEnvMap));
}

Sky::~Sky()
{
	ReleaseCOM(mSphere);
	ReleaseCOM(mEnvMap);
	ReleaseCOM(mFX);
}

IDirect3DCubeTexture9 * Sky::GetEnvMap()
{
	return mEnvMap;
}

float Sky::GetRadius()
{
	return mRadius;
}

DWORD Sky::GetNumTriangles()
{
	return mSphere->GetNumFaces();
}

DWORD Sky::GetNumVertices()
{
	return mSphere->GetNumVertices();
}

void Sky::OnLostDevice()
{
	HR(mFX->OnLostDevice());
}

void Sky::OnResetDevice()
{
	HR(mFX->OnResetDevice());
}

void Sky::Draw()
{
	D3DXMATRIX W;
	D3DXVECTOR3 p = gCamera->Pos();
	D3DXMatrixTranslation(&W, p.x, p.y, p.z);
	HR(mFX->SetMatrix(mhWVP, &(W*gCamera->ViewProj())));

	UINT numPasses = 0;
	HR(mFX->Begin(&numPasses, 0));
	HR(mFX->BeginPass(0));
	HR(mSphere->DrawSubset(0));
	HR(mFX->EndPass());
	HR(mFX->End());
}
