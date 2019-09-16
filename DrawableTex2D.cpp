#include "DrawableTex2D.h"
#include "d3dApp.h"

DrawableTex2D::DrawableTex2D(UINT width, UINT height, UINT mipLevels,
	D3DFORMAT texFormat, bool useDepthBuffer,
	D3DFORMAT depthFormat, D3DVIEWPORT9& viewport, bool autoGenMips)
	: mTex(0), mRTS(0), mTopSurf(0), mWidth(width), mHeight(height),
	mMipLevels(mipLevels), mTexFormat(texFormat), mUseDepthBuffer(useDepthBuffer),
	mDepthFormat(depthFormat), mViewPort(viewport), mAutoGenMips(autoGenMips)
{
}

DrawableTex2D::~DrawableTex2D()
{
	OnLostDevice();
}

IDirect3DTexture9* DrawableTex2D::D3dTex()
{
	return mTex;
}

void DrawableTex2D::OnLostDevice()
{
	ReleaseCOM(mTex);
	ReleaseCOM(mRTS);
	ReleaseCOM(mTopSurf);
}

void DrawableTex2D::OnResetDevice()
{
	UINT usage = D3DUSAGE_RENDERTARGET;
	if (mAutoGenMips)
		usage |= D3DUSAGE_AUTOGENMIPMAP;

	HR(D3DXCreateTexture(gD3dDevice, mWidth, mHeight, mMipLevels, usage, mTexFormat, D3DPOOL_DEFAULT, &mTex));
	HR(D3DXCreateRenderToSurface(gD3dDevice, mWidth, mHeight, mTexFormat, mUseDepthBuffer, mDepthFormat, &mRTS));
	HR(mTex->GetSurfaceLevel(0, &mTopSurf));
}

void DrawableTex2D::BeginScene()
{
	mRTS->BeginScene(mTopSurf, &mViewPort);
}

void DrawableTex2D::EndScene()
{
	mRTS->EndScene(D3DX_FILTER_NONE);
}
