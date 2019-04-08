#include "Vertex.h"
#include "DirectInput.h"
#include "CloudDemoApp.h"

CloudDemoApp::CloudDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP) : D3DApp(hInstance, winCaption, devType, requestedVP)
{
	if (!CheckDeviceCaps())
	{
		MessageBox(0, "checkDeviceCaps() Failed", 0, 0);
		PostQuitMessage(0);
	}

	mGfxStats = new GfxStats();

	mCameraRadius = 30.0f;
	mCameraRotationY = 1.2 * D3DX_PI;
	mCameraHeight = 15.0f;

	D3DXMatrixIdentity(&mWorld);

	HR(D3DXCreateTextureFromFile(gD3dDevice, "cloud0.dds", &mCloudTex0));
	HR(D3DXCreateTextureFromFile(gD3dDevice, "cloud1.dds", &mCloudTex1));

	mTexOffset0 = D3DXVECTOR2(0.0f, 0.0f);
	mTexOffset1 = D3DXVECTOR2(0.0f, 0.0f);

	BuildGridGeometry();
	mGfxStats->AddVertices(mNumGridVertices);
	mGfxStats->AddTriangles(mNumGridTriangles);

	BuildFx();

	OnResetDevice();

	InitAllVertexDeclarations();
}

CloudDemoApp::~CloudDemoApp()
{
	delete mGfxStats;
	ReleaseCOM(mGridVB);
	ReleaseCOM(mGridIB);
	ReleaseCOM(mCloudTex0);
	ReleaseCOM(mCloudTex1);
	ReleaseCOM(mFX);

	DestroyAllVertexDeclarations();
}

bool CloudDemoApp::CheckDeviceCaps()
{
	D3DCAPS9 caps;
	HR(gD3dDevice->GetDeviceCaps(&caps));

	if (caps.VertexShaderVersion < D3DVS_VERSION(2, 0))
		return false;

	if (caps.PixelShaderVersion < D3DPS_VERSION(2, 0))
		return false;

	return true;
}

void CloudDemoApp::OnLostDevice()
{
	mGfxStats->OnLostDevice();
	HR(mFX->OnLostDevice());
}

void CloudDemoApp::OnResetDevice()
{
	mGfxStats->OnResetDevice();
	HR(mFX->OnResetDevice());

	BuildProjMtx();
}

void CloudDemoApp::UpdateScene(float dt)
{
	mGfxStats->Update(dt);

	gDInput->poll();

	// Check input.
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

	mTexOffset0 += D3DXVECTOR2(0.11f, 0.05f) * dt;
	mTexOffset1 += D3DXVECTOR2(0.25f, 0.1f) * dt;

	if (mTexOffset0.x >= 1.0f || mTexOffset0.x <= -1.0f)
		mTexOffset0.x = 0.0f;
	if (mTexOffset1.x >= 1.0f || mTexOffset1.x <= -1.0f)
		mTexOffset1.x = 0.0f;
	if (mTexOffset0.y >= 1.0f || mTexOffset0.y <= -1.0f)
		mTexOffset0.y = 0.0f;
	if (mTexOffset1.y >= 1.0f || mTexOffset1.y <= -1.0f)
		mTexOffset1.y = 0.0f;
}

void CloudDemoApp::DrawScene()
{
	HR(gD3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffeeeeee, 1.0f, 0));
	HR(gD3dDevice->BeginScene());

	// Setup the rendering FX
	HR(mFX->SetTechnique(mhTech));

	HR(mFX->SetMatrix(mhWVP, &(mWorld*mView*mProj)));
	HR(mFX->SetTexture(mhCloudTex0, mCloudTex0));
	HR(mFX->SetTexture(mhCloudTex1, mCloudTex1));
	HR(mFX->SetValue(mhTexOffset0, &mTexOffset0, sizeof(D3DXVECTOR2)));
	HR(mFX->SetValue(mhTexOffset1, &mTexOffset1, sizeof(D3DXVECTOR2)));

	HR(gD3dDevice->SetVertexDeclaration(VertexPT::decl));
	HR(gD3dDevice->SetStreamSource(0, mGridVB, 0, sizeof(VertexPT)));
	HR(gD3dDevice->SetIndices(mGridIB));

	UINT numPasses = 0;
	HR(mFX->Begin(&numPasses, 0));
	for (UINT i = 0; i < numPasses; ++i)
	{
		HR(mFX->BeginPass(i));
		HR(gD3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, mNumGridVertices, 0, mNumGridTriangles));
		HR(mFX->EndPass());
	}
	HR(mFX->End());


	mGfxStats->Display();

	HR(gD3dDevice->EndScene());

	HR(gD3dDevice->Present(0, 0, 0, 0));
}

void CloudDemoApp::BuildGridGeometry()
{
	std::vector<D3DXVECTOR3> verts;
	std::vector<DWORD> indices;

	GenTriGrid(100, 100, 1.0f, 1.0f, D3DXVECTOR3(0.0f, 0.0f, 0.0f), verts, indices);

	mNumGridVertices = 100 * 100;
	mNumGridTriangles = 99 * 99 * 2;

	HR(gD3dDevice->CreateVertexBuffer(mNumGridVertices * sizeof(VertexPT), D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &mGridVB, 0));

	VertexPT* v = 0;
	HR(mGridVB->Lock(0, 0, (void**)&v, 0));

	float texScale = 0.02f;
	for (int i = 0; i < 100; ++i)
	{
		for (int j = 0; j < 100; ++j)
		{
			DWORD index = i * 100 + j;
			v[index].pos = verts[index];
			v[index].tex0 = D3DXVECTOR2((float)j, (float)i) * texScale;
		}
	}

	HR(mGridVB->Unlock());

	HR(gD3dDevice->CreateIndexBuffer(mNumGridTriangles * 3 * sizeof(WORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &mGridIB, 0));

	WORD* k = 0;
	HR(mGridIB->Lock(0, 0, (void**)&k, 0));

	for (DWORD i = 0; i < mNumGridTriangles * 3; ++i)
		k[i] = (WORD)indices[i];

	HR(mGridIB->Unlock());
}

void CloudDemoApp::BuildFx()
{
	ID3DXBuffer* errors = 0;
	HR(D3DXCreateEffectFromFile(gD3dDevice, "clouds.fx", 0, 0, D3DXSHADER_DEBUG, 0, &mFX, &errors));
	if (errors)
		MessageBox(0, (char*)errors->GetBufferPointer(), 0, 0);

	mhTech = mFX->GetTechniqueByName("CloudsTech");
	mhWVP = mFX->GetParameterByName(0, "gWVP");
	mhCloudTex0 = mFX->GetParameterByName(0, "gCloudTex0");
	mhCloudTex1 = mFX->GetParameterByName(0, "gCloudTex1");
	mhTexOffset0 = mFX->GetParameterByName(0, "gTexOffset0");
	mhTexOffset1 = mFX->GetParameterByName(0, "gTexOffset1");
}

void CloudDemoApp::BuildViewMtx()
{
	float x = mCameraRadius * cosf(mCameraRotationY);
	float z = mCameraRadius * sinf(mCameraRotationY);
	D3DXVECTOR3 pos(x, mCameraHeight, z);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&mView, &pos, &target, &up);
}

void CloudDemoApp::BuildProjMtx()
{
	float w = (float)mD3dpp.BackBufferWidth;
	float h = (float)mD3dpp.BackBufferHeight;
	D3DXMatrixPerspectiveFovLH(&mProj, D3DX_PI * 0.25f, w / h, 1.0f, 5000.0f);
}
