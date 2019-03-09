#include "TriGridDemoApp.h"
#include "Vertex.h"
#include "DirectInput.h"
#include <vector>


TriGridDemoApp::TriGridDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP) : D3DApp(hInstance, winCaption, devType, requestedVP)
{
	if (!CheckDeviceCaps())
	{
		MessageBox(0, "checkDeviceCaps() Failed", 0, 0);
		PostQuitMessage(0);
	}

	mGfxStats = new GfxStats();
	mCameraRadius = 10.0f;
	mCameraRotationY = 1.2 * D3DX_PI;
	mCameraHeight = 5.0f;

	BuildGeoBuffers();
	BuildFx();

	OnResetDevice();

	InitAllVertexDeclarations();
}

TriGridDemoApp::~TriGridDemoApp()
{
	delete mGfxStats;
	ReleaseCOM(mVb);
	ReleaseCOM(mIb);
	ReleaseCOM(mFx);
	
	DestroyAllVertexDeclarations();
}

bool TriGridDemoApp::CheckDeviceCaps()
{
	D3DCAPS9 caps;
	HR(gD3dDevice->GetDeviceCaps(&caps));
	if (caps.VertexShaderVersion < D3DVS_VERSION(2, 0))
		return false;
	if (caps.PixelShaderVersion < D3DPS_VERSION(2, 0))
		return false;
	return true;
}

void TriGridDemoApp::OnLostDevice()
{
	mGfxStats->OnLostDevice();
	HR(mFx->OnLostDevice());
}

void TriGridDemoApp::OnResetDevice()
{
	mGfxStats->OnResetDevice();
	HR(mFx->OnResetDevice());
	BuildProjMtx();
}

void TriGridDemoApp::UpdateScene(float dt)
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

	BuildViewMtx();
}

void TriGridDemoApp::DrawScene()
{
	HR(gD3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0));
	HR(gD3dDevice->BeginScene());
	HR(gD3dDevice->SetStreamSource(0, mVb, 0, sizeof(VertexPos)));
	HR(gD3dDevice->SetIndices(mIb));
	HR(gD3dDevice->SetVertexDeclaration(VertexPos::decl));
	HR(mFx->SetTechnique(mhTech));
	HR(mFx->SetMatrix(mhWVP, &(mView * mProj)));

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

void TriGridDemoApp::BuildGeoBuffers()
{
	std::vector<D3DXVECTOR3> verts;
	std::vector<DWORD> indices;

	GenTriGrid(100, 100, 1.0f, 1.0f, D3DXVECTOR3(0.0f, 0.0f, 0.0f), verts, indices);

	mNumVertices = 100 * 100;
	mNumTriangles = 99 * 99 * 2;

	HR(gD3dDevice->CreateVertexBuffer(mNumVertices * sizeof(VertexPos),
		D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &mVb, 0));

	VertexPos* v = 0;
	HR(mVb->Lock(0, 0, (void**)&v, 0));

	for (DWORD i = 0; i < mNumVertices; ++i)
		v[i] = verts[i];

	HR(mVb->Unlock());


	HR(gD3dDevice->CreateIndexBuffer(mNumTriangles * 3 * sizeof(WORD), D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16, D3DPOOL_MANAGED, &mIb, 0));


	WORD* k = 0;
	HR(mIb->Lock(0, 0, (void**)&k, 0));

	for (DWORD i = 0; i < mNumTriangles * 3; ++i)
		k[i] = (WORD)indices[i];

	HR(mIb->Unlock());
}

void TriGridDemoApp::BuildFx()
{
	ID3DXBuffer *errors = NULL;
	HR(D3DXCreateEffectFromFile(gD3dDevice, "transform.fx", 0, 0, D3DXSHADER_DEBUG | D3DXSHADER_ENABLE_BACKWARDS_COMPATIBILITY, 0, &mFx, &errors));
	if(errors)
		MessageBox(0, (char*)errors->GetBufferPointer(), 0, 0);

	mhTech = mFx->GetTechniqueByName("TransformTech");
	mhWVP = mFx->GetParameterByName(0, "gWVP");
}

void TriGridDemoApp::BuildViewMtx()
{
	float x = mCameraRadius * cosf(mCameraRotationY);
	float z = mCameraRadius * sinf(mCameraRotationY);
	D3DXVECTOR3 pos(x, mCameraHeight, z);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&mView, &pos, &target, &up);
}

void TriGridDemoApp::BuildProjMtx()
{
	float w = (float)mD3dpp.BackBufferWidth;
	float h = (float)mD3dpp.BackBufferHeight;
	D3DXMatrixPerspectiveFovLH(&mProj, D3DX_PI * 0.25f, w / h, 1.0f, 5000.0f);
}
