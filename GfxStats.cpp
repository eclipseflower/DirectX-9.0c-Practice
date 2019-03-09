#include "GfxStats.h"
#include "D3DUtil.h"
#include "D3DApp.h"
#include <tchar.h>

GfxStats::GfxStats() : mFont(0), mFPS(0.0f), mMilliSecPerFrame(0.0f), mNumTris(0), mNumVertices(0)
{
	D3DXFONT_DESC fontDesc;
	fontDesc.Height = 18;
	fontDesc.Width = 0;
	fontDesc.Weight = 0;
	fontDesc.MipLevels = 1;
	fontDesc.Italic = false;
	fontDesc.CharSet = DEFAULT_CHARSET;
	fontDesc.OutputPrecision = OUT_DEFAULT_PRECIS;
	fontDesc.Quality = DEFAULT_QUALITY;
	fontDesc.PitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	strcpy_s(fontDesc.FaceName, _T("Times New Roman"));

	HR(D3DXCreateFontIndirect(gD3dDevice, &fontDesc, &mFont));
}


GfxStats::~GfxStats()
{
	ReleaseCOM(mFont);
}

void GfxStats::OnLostDevice()
{
	HR(mFont->OnLostDevice());
}

void GfxStats::OnResetDevice()
{
	HR(mFont->OnResetDevice());
}

void GfxStats::AddVertices(DWORD n)
{
	mNumVertices += n;
}

void GfxStats::SubVertices(DWORD n)
{
	mNumVertices -= n;
}

void GfxStats::AddTriangles(DWORD n)
{
	mNumTris += n;
}

void GfxStats::SubTriangles(DWORD n)
{
	mNumTris -= n;
}

void GfxStats::SetTriCount(DWORD n)
{
	mNumTris = n;
}

void GfxStats::SetVertexCount(DWORD n)
{
	mNumVertices = n;
}

void GfxStats::Update(float dt)
{
	static float numFrames = 0.0f;
	static float timeElapsed = 0.0f;

	numFrames += 1.0f;
	timeElapsed += dt;

	if (timeElapsed >= 1.0f)
	{
		mFPS = numFrames;
		mMilliSecPerFrame = 1000.0f / mFPS;
		timeElapsed = 0.0f;
		numFrames = 0.0f;
	}
}

void GfxStats::Display()
{
	static char buffer[256];
	sprintf_s(buffer, "Frames Per Second = %.2f\n"
		"Milliseconds Per Frame = %.4f\n"
		"Triangle Count = %d\n"
		"Vertex Count = %d", mFPS, mMilliSecPerFrame, mNumTris, mNumVertices);
	RECT R = { 5, 5, 0, 0 };
	HR(mFont->DrawText(0, buffer, -1, &R, DT_NOCLIP, D3DCOLOR_XRGB(0, 0, 0)));
}