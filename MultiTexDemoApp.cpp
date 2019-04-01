#include "DirectInput.h"
#include "Vertex.h"
#include "MultiTexDemoApp.h"

MultiTexDemoApp::MultiTexDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP) : D3DApp(hInstance, winCaption, devType, requestedVP)
{
	if (!CheckDeviceCaps())
	{
		MessageBox(0, "checkDeviceCaps() Failed", 0, 0);
		PostQuitMessage(0);
	}

	mGfxStats = new GfxStats();

	mCameraRadius = 6.0f;
	mCameraRotationY = 1.2 * D3DX_PI;
	mCameraHeight = 3.0f;

	mLightVecW = D3DXVECTOR3(0.0, 0.707f, -0.707f);
	mDiffuseMtrl = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	mDiffuseLight = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	mAmbientMtrl = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	mAmbientLight = D3DXCOLOR(0.6f, 0.6f, 0.6f, 1.0f);
	mSpecularMtrl = D3DXCOLOR(0.4f, 0.4f, 0.4f, 1.0f);
	mSpecularLight = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	mSpecularPower = 8.0f;

	D3DXMatrixIdentity(&mWorld);

	// Load textures from file.
	HR(D3DXCreateTextureFromFile(gD3dDevice, "grass0.dds", &mTex0));
	HR(D3DXCreateTextureFromFile(gD3dDevice, "stone2.dds", &mTex1));
	HR(D3DXCreateTextureFromFile(gD3dDevice, "ground0.dds", &mTex2));
	HR(D3DXCreateTextureFromFile(gD3dDevice, "blendmap.jpg", &mBlendMap));

	BuildGridGeometry();
	mGfxStats->AddVertices(mNumGridVertices);
	mGfxStats->AddTriangles(mNumGridTriangles);

	BuildFx();

	OnResetDevice();

	InitAllVertexDeclarations();
}

MultiTexDemoApp::~MultiTexDemoApp()
{
	delete mGfxStats;
	ReleaseCOM(mGridVB);
	ReleaseCOM(mGridIB);
	ReleaseCOM(mTex0);
	ReleaseCOM(mTex1);
	ReleaseCOM(mTex2);
	ReleaseCOM(mBlendMap);
	ReleaseCOM(mFX);

	DestroyAllVertexDeclarations();
}

bool MultiTexDemoApp::CheckDeviceCaps()
{
	D3DCAPS9 caps;
	HR(gD3dDevice->GetDeviceCaps(&caps));

	if (caps.VertexShaderVersion < D3DVS_VERSION(2, 0))
		return false;

	if (caps.PixelShaderVersion < D3DPS_VERSION(2, 0))
		return false;

	return true;
}

void MultiTexDemoApp::OnLostDevice()
{
	mGfxStats->OnLostDevice();
	HR(mFX->OnLostDevice());
}

void MultiTexDemoApp::OnResetDevice()
{
	mGfxStats->OnResetDevice();
	HR(mFX->OnResetDevice());

	BuildProjMtx();
}

void MultiTexDemoApp::UpdateScene(float dt)
{
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

void MultiTexDemoApp::DrawScene()
{
	HR(gD3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffeeeeee, 1.0f, 0));

	HR(gD3dDevice->BeginScene());

	HR(mFX->SetTechnique(mhTech));

	HR(mFX->SetMatrix(mhWVP, &(mWorld*mView*mProj)));
	D3DXMATRIX worldInvTrans;
	D3DXMatrixInverse(&worldInvTrans, 0, &mWorld);
	D3DXMatrixTranspose(&worldInvTrans, &worldInvTrans);
	HR(mFX->SetMatrix(mhWorldInvTrans, &worldInvTrans));
	HR(mFX->SetValue(mhLightVecW, &mLightVecW, sizeof(D3DXVECTOR3)));
	HR(mFX->SetValue(mhDiffuseMtrl, &mDiffuseMtrl, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhDiffuseLight, &mDiffuseLight, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhAmbientMtrl, &mAmbientMtrl, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhAmbientLight, &mAmbientLight, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhSpecularLight, &mSpecularLight, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhSpecularMtrl, &mSpecularMtrl, sizeof(D3DXCOLOR)));
	HR(mFX->SetFloat(mhSpecularPower, mSpecularPower));
	HR(mFX->SetMatrix(mhWorld, &mWorld));
	HR(mFX->SetTexture(mhTex0, mTex0));
	HR(mFX->SetTexture(mhTex1, mTex1));
	HR(mFX->SetTexture(mhTex2, mTex2));
	HR(mFX->SetTexture(mhBlendMap, mBlendMap));

	HR(gD3dDevice->SetVertexDeclaration(VertexPNT::decl));
	HR(gD3dDevice->SetStreamSource(0, mGridVB, 0, sizeof(VertexPNT)));
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

void MultiTexDemoApp::BuildGridGeometry()
{
	std::vector<D3DXVECTOR3> verts;
	std::vector<DWORD> indices;

	GenTriGrid(100, 100, 1.0f, 1.0f, D3DXVECTOR3(0.0f, 0.0f, 0.0f), verts, indices);

	mNumGridVertices = 100 * 100;
	mNumGridTriangles = 99 * 99 * 2;

	HR(gD3dDevice->CreateVertexBuffer(mNumGridVertices * sizeof(VertexPNT), D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &mGridVB, 0));

	VertexPNT* v = 0;
	HR(mGridVB->Lock(0, 0, (void**)&v, 0));

	float w = 99.0f;
	float d = 99.0f;
	for (int i = 0; i < 100; ++i)
	{
		for (int j = 0; j < 100; ++j)
		{
			DWORD index = i * 100 + j;
			v[index].pos = verts[index];
			v[index].normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
			v[index].tex0.x = (v[index].pos.x + (0.5f*w)) / w;
			v[index].tex0.y = (v[index].pos.z - (0.5f*d)) / -d;
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

void MultiTexDemoApp::BuildFx()
{
	ID3DXBuffer* errors = 0;
	HR(D3DXCreateEffectFromFile(gD3dDevice, "terrainmultitex.fx", 0, 0, D3DXSHADER_DEBUG, 0, &mFX, &errors));
	if (errors)
		MessageBox(0, (char*)errors->GetBufferPointer(), 0, 0);

	mhTech = mFX->GetTechniqueByName("TerrainMultiTexTech");
	mhWVP = mFX->GetParameterByName(0, "gWVP");
	mhWorldInvTrans = mFX->GetParameterByName(0, "gWorldInvTrans");
	mhLightVecW = mFX->GetParameterByName(0, "gLightVecW");
	mhDiffuseMtrl = mFX->GetParameterByName(0, "gDiffuseMtrl");
	mhDiffuseLight = mFX->GetParameterByName(0, "gDiffuseLight");
	mhAmbientMtrl = mFX->GetParameterByName(0, "gAmbientMtrl");
	mhAmbientLight = mFX->GetParameterByName(0, "gAmbientLight");
	mhSpecularMtrl = mFX->GetParameterByName(0, "gSpecularMtrl");
	mhSpecularLight = mFX->GetParameterByName(0, "gSpecularLight");
	mhSpecularPower = mFX->GetParameterByName(0, "gSpecularPower");
	mhEyePos = mFX->GetParameterByName(0, "gEyePosW");
	mhWorld = mFX->GetParameterByName(0, "gWorld");
	mhTex0 = mFX->GetParameterByName(0, "gTex0");
	mhTex1 = mFX->GetParameterByName(0, "gTex1");
	mhTex2 = mFX->GetParameterByName(0, "gTex2");
	mhBlendMap = mFX->GetParameterByName(0, "gBlendMap");
}

void MultiTexDemoApp::BuildViewMtx()
{
	float x = mCameraRadius * cosf(mCameraRotationY);
	float z = mCameraRadius * sinf(mCameraRotationY);
	D3DXVECTOR3 pos(x, mCameraHeight, z);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&mView, &pos, &target, &up);

	HR(mFX->SetValue(mhEyePos, &pos, sizeof(D3DXVECTOR3)));
}

void MultiTexDemoApp::BuildProjMtx()
{
	float w = (float)mD3dpp.BackBufferWidth;
	float h = (float)mD3dpp.BackBufferHeight;
	D3DXMatrixPerspectiveFovLH(&mProj, D3DX_PI * 0.25f, w / h, 1.0f, 5000.0f);
}
