#pragma once
#include <d3dx9.h>

class GfxStats
{
public:
	GfxStats();
	~GfxStats();

	void OnLostDevice();
	void OnResetDevice();

	void AddVertices(DWORD n);
	void SubVertices(DWORD n);
	void AddTriangles(DWORD n);
	void SubTriangles(DWORD n);

	void SetTriCount(DWORD n);
	void SetVertexCount(DWORD n);

	void Update(float dt);
	void Display();

private:
	GfxStats(const GfxStats &rhs);
	GfxStats& operator = (const GfxStats &rhs);

	ID3DXFont *mFont;
	float mFPS;
	float mMilliSecPerFrame;
	DWORD mNumTris;
	DWORD mNumVertices;
};

