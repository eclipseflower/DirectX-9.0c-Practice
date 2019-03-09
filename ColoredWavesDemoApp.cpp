#include "Vertex.h"
#include "DirectInput.h"
#include "ColoredWavesDemoApp.h"

ColoredWavesDemoApp::ColoredWavesDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP) : D3DApp(hInstance, winCaption, devType, requestedVP)
{
	if (!CheckDeviceCaps())
	{
		MessageBox(0, "checkDeviceCaps() Failed", 0, 0);
		PostQuitMessage(0);
	}

	mGfxStats = new GfxStats();

	mCameraRadius = 25.0f;
	mCameraRotationY = 1.2 * D3DX_PI;
	mCameraHeight = 15.0f;
	mTime = 0.0f;

	BuildGeoBuffers();
	BuildFx();

	OnResetDevice();

	InitAllVertexDeclarations();
}

ColoredWavesDemoApp::~ColoredWavesDemoApp()
{
	delete mGfxStats;
	ReleaseCOM(mVb);
	ReleaseCOM(mIb);
	ReleaseCOM(mFx);
	DestroyAllVertexDeclarations();
}

bool ColoredWavesDemoApp::CheckDeviceCaps()
{
	D3DCAPS9 caps;
	HR(gD3dDevice->GetDeviceCaps(&caps));

	if (caps.VertexShaderVersion < D3DVS_VERSION(2, 0))
		return false;

	if (caps.PixelShaderVersion < D3DPS_VERSION(2, 0))
		return false;

	return true;
}

void ColoredWavesDemoApp::OnLostDevice()
{
	mGfxStats->OnLostDevice();
	HR(mFx->OnLostDevice());
}

void ColoredWavesDemoApp::OnResetDevice()
{
	mGfxStats->OnResetDevice();
	HR(mFx->OnResetDevice());

	BuildProjMtx();
}

void ColoredWavesDemoApp::UpdateScene(float dt)
{
	mGfxStats->SetVertexCount(mNumVertices);
	mGfxStats->SetTriCount(mNumTriangles);
	mGfxStats->Update(dt);

	gDInput->poll();

	if (gDInput->keyDown(DIK_W))
		mCameraHeight += 25.0f * dt;
	if (gDInput->keyDown(DIK_S))
		mCameraHeight -= 25.0f * dt;

	mCameraRotationY += gDInput->mouseDX() / 100.0f;
	mCameraRadius += gDInput->mouseDY() / 25.0f;

	if (fabsf(mCameraRotationY) >= 2.0f * D3DX_PI)
		mCameraRotationY = 0.0f;

	if (mCameraRadius < 5.0f)
		mCameraRadius = 5.0f;

	mTime += dt;

	BuildViewMtx();
}

void ColoredWavesDemoApp::DrawScene()
{
	HR(gD3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0));

	HR(gD3dDevice->BeginScene());

	HR(gD3dDevice->SetStreamSource(0, mVb, 0, sizeof(VertexPos)));
	HR(gD3dDevice->SetIndices(mIb));
	HR(gD3dDevice->SetVertexDeclaration(VertexPos::decl));

	// Setup the rendering FX
	HR(mFx->SetTechnique(mhTech));
	HR(mFx->SetMatrix(mhWVP, &(mView*mProj)));
	HR(mFx->SetFloat(mhTime, mTime));

	UINT numPasses = 0;
	HR(mFx->Begin(&numPasses, 0));
	for (UINT i = 0; i < numPasses; ++i)
	{
		HR(mFx->BeginPass(i));
		HR(gD3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, mNumVertices, 0, mNumTriangles));
		HR(mFx->EndPass());
	}
	HR(mFx->End());


	mGfxStats->Display();

	HR(gD3dDevice->EndScene());

	HR(gD3dDevice->Present(0, 0, 0, 0));
}

void ColoredWavesDemoApp::BuildGeoBuffers()
{
	std::vector<D3DXVECTOR3> verts;
	std::vector<DWORD> indices;

	GenTriGrid(100, 100, 0.5f, 0.5f, D3DXVECTOR3(0.0f, 0.0f, 0.0f), verts, indices);

	mNumVertices = 100 * 100;
	mNumTriangles = 99 * 99 * 2;

	HR(gD3dDevice->CreateVertexBuffer(mNumVertices * sizeof(VertexPos), D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &mVb, 0));

	VertexPos* v = NULL;
	HR(mVb->Lock(0, 0, (void**)&v, 0));

	for (DWORD i = 0; i < mNumVertices; ++i)
		v[i] = verts[i];

	HR(mVb->Unlock());

	HR(gD3dDevice->CreateIndexBuffer(mNumTriangles * 3 * sizeof(WORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &mIb, 0));

	WORD* k = NULL;
	HR(mIb->Lock(0, 0, (void**)&k, 0));

	for (DWORD i = 0; i < mNumTriangles * 3; ++i)
		k[i] = (WORD)indices[i];

	HR(mIb->Unlock());
}

void ColoredWavesDemoApp::BuildFx()
{
	ID3DXBuffer* errors = 0;
	HR(D3DXCreateEffectFromFile(gD3dDevice, "heightcolor.fx", 0, 0, D3DXSHADER_DEBUG, 0, &mFx, &errors));
	if (errors)
		MessageBox(0, (char*)errors->GetBufferPointer(), 0, 0);

	mhTech = mFx->GetTechniqueByName("HeightColorTech");
	mhWVP = mFx->GetParameterByName(0, "gWVP");
	mhTime = mFx->GetParameterByName(0, "gTime");
}

void ColoredWavesDemoApp::BuildViewMtx()
{
	float x = mCameraRadius * cosf(mCameraRotationY);
	float z = mCameraRadius * sinf(mCameraRotationY);
	D3DXVECTOR3 pos(x, mCameraHeight, z);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&mView, &pos, &target, &up);
}

void ColoredWavesDemoApp::BuildProjMtx()
{
	float w = (float)mD3dpp.BackBufferWidth;
	float h = (float)mD3dpp.BackBufferHeight;
	D3DXMatrixPerspectiveFovLH(&mProj, D3DX_PI * 0.25f, w / h, 1.0f, 5000.0f);
}
