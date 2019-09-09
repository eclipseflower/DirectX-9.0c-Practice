#pragma once

#include <d3dx9.h>
#include <string>

class Sky
{
public:
	Sky(const std::string& envmapFilename, float skyRadius);
	~Sky();

	IDirect3DCubeTexture9* GetEnvMap();
	float GetRadius();

	DWORD GetNumTriangles();
	DWORD GetNumVertices();

	void OnLostDevice();
	void OnResetDevice();

	void Draw();

private:
	ID3DXMesh* mSphere;
	float mRadius;
	IDirect3DCubeTexture9* mEnvMap;
	ID3DXEffect* mFX;
	D3DXHANDLE mhTech;
	D3DXHANDLE mhEnvMap;
	D3DXHANDLE mhWVP;
};

