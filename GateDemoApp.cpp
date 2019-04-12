#include "DirectInput.h"
#include "Vertex.h"
#include "GateDemoApp.h"


GateDemoApp::GateDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP) : D3DApp(hInstance, winCaption, devType, requestedVP)
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
	mDiffuseLight = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	mAmbientLight = D3DXCOLOR(0.6f, 0.6f, 0.6f, 1.0f);
	mSpecularLight = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

	mGroundMtrl.ambient = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	mGroundMtrl.diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	mGroundMtrl.spec = D3DXCOLOR(0.4f, 0.4f, 0.4f, 1.0f);
	mGroundMtrl.specPower = 8.0f;

	mGateMtrl.ambient = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	mGateMtrl.diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	mGateMtrl.spec = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
	mGateMtrl.specPower = 8.0f;

	D3DXMatrixIdentity(&mGroundWorld);
	D3DXMatrixIdentity(&mGateWorld);

	HR(D3DXCreateTextureFromFile(gD3dDevice, "ground0.dds", &mGroundTex));
	HR(D3DXCreateTextureFromFile(gD3dDevice, "gatea.dds", &mGateTex));

	BuildGridGeometry();
	BuildGateGeometry();

	mGfxStats->AddVertices(mNumGridVertices);
	mGfxStats->AddTriangles(mNumGridTriangles);

	mGfxStats->AddVertices(4);
	mGfxStats->AddTriangles(2);

	BuildFX();

	OnResetDevice();

	InitAllVertexDeclarations();
}

GateDemoApp::~GateDemoApp()
{
	delete mGfxStats;
	ReleaseCOM(mGridVB);
	ReleaseCOM(mGridIB);
	ReleaseCOM(mGroundTex);
	ReleaseCOM(mGateVB);
	ReleaseCOM(mGateIB);
	ReleaseCOM(mGateTex);
	ReleaseCOM(mFX);

	DestroyAllVertexDeclarations();
}

bool GateDemoApp::CheckDeviceCaps()
{
	D3DCAPS9 caps;
	HR(gD3dDevice->GetDeviceCaps(&caps));

	if (caps.VertexShaderVersion < D3DVS_VERSION(2, 0))
		return false;

	if (caps.PixelShaderVersion < D3DPS_VERSION(2, 0))
		return false;

	return true;
}

void GateDemoApp::OnLostDevice()
{
	mGfxStats->OnLostDevice();
	HR(mFX->OnLostDevice());
}

void GateDemoApp::OnResetDevice()
{
	mGfxStats->OnResetDevice();
	HR(mFX->OnResetDevice());

	BuildProjMtx();
}

void GateDemoApp::UpdateScene(float dt)
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
}

void GateDemoApp::DrawScene()
{
	HR(gD3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffeeeeee, 1.0f, 0));

	HR(gD3dDevice->BeginScene());

	HR(mFX->SetTechnique(mhTech));
	HR(mFX->SetValue(mhLightVecW, &mLightVecW, sizeof(D3DXVECTOR3)));
	HR(mFX->SetValue(mhDiffuseLight, &mDiffuseLight, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhAmbientLight, &mAmbientLight, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhSpecularLight, &mSpecularLight, sizeof(D3DXCOLOR)));

	DrawGround();
	DrawGate();

	mGfxStats->Display();

	HR(gD3dDevice->EndScene());

	HR(gD3dDevice->Present(0, 0, 0, 0));
}

void GateDemoApp::BuildGridGeometry()
{
	std::vector<D3DXVECTOR3> verts;
	std::vector<DWORD> indices;

	GenTriGrid(100, 100, 1.0f, 1.0f, D3DXVECTOR3(0.0f, 0.0f, 0.0f), verts, indices);

	mNumGridVertices = 100 * 100;
	mNumGridTriangles = 99 * 99 * 2;

	HR(gD3dDevice->CreateVertexBuffer(mNumGridVertices * sizeof(VertexPNT), D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &mGridVB, 0));

	VertexPNT* v = NULL;
	HR(mGridVB->Lock(0, 0, (void**)&v, 0));

	float texScale = 0.2f;
	for (int i = 0; i < 100; ++i)
	{
		for (int j = 0; j < 100; ++j)
		{
			DWORD index = i * 100 + j;
			v[index].pos = verts[index];
			v[index].normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
			v[index].tex0 = D3DXVECTOR2((float)j, (float)i) * texScale;
		}
	}

	HR(mGridVB->Unlock());

	HR(gD3dDevice->CreateIndexBuffer(mNumGridTriangles * 3 * sizeof(WORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &mGridIB, 0));

	WORD* k = NULL;
	HR(mGridIB->Lock(0, 0, (void**)&k, 0));

	for (DWORD i = 0; i < mNumGridTriangles * 3; ++i)
		k[i] = (WORD)indices[i];

	HR(mGridIB->Unlock());
}

void GateDemoApp::BuildGateGeometry()
{
	HR(gD3dDevice->CreateVertexBuffer(4 * sizeof(VertexPNT), D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &mGateVB, 0));

	VertexPNT* v = NULL;
	HR(mGateVB->Lock(0, 0, (void**)&v, 0));

	v[0] = VertexPNT(-20.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[1] = VertexPNT(-20.0f, 5.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[2] = VertexPNT(20.0f, 5.0f, 0.0f, 0.0f, 0.0f, -1.0f, 4.0f, 0.0f);
	v[3] = VertexPNT(20.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 4.0f, 1.0f);
	HR(mGateVB->Unlock());

	HR(gD3dDevice->CreateIndexBuffer(6 * sizeof(WORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &mGateIB, 0));

	WORD* k = NULL;
	HR(mGateIB->Lock(0, 0, (void**)&k, 0));

	k[0] = 0;  k[1] = 1;  k[2] = 2; // Triangle 0
	k[3] = 0;  k[4] = 2;  k[5] = 3; // Triangle 1

	HR(mGateIB->Unlock());
}

void GateDemoApp::BuildFX()
{
	ID3DXBuffer* errors = 0;
	HR(D3DXCreateEffectFromFile(gD3dDevice, "DirLightTex.fx", 0, 0, D3DXSHADER_DEBUG, 0, &mFX, &errors));
	if (errors)
		MessageBox(0, (char*)errors->GetBufferPointer(), 0, 0);

	mhTech = mFX->GetTechniqueByName("DirLightTexTech");
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
	mhTex = mFX->GetParameterByName(0, "gTex");
}

void GateDemoApp::BuildViewMtx()
{
	float x = mCameraRadius * cosf(mCameraRotationY);
	float z = mCameraRadius * sinf(mCameraRotationY);
	D3DXVECTOR3 pos(x, mCameraHeight, z);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&mView, &pos, &target, &up);

	HR(mFX->SetValue(mhEyePos, &pos, sizeof(D3DXVECTOR3)));
}

void GateDemoApp::BuildProjMtx()
{
	float w = (float)mD3dpp.BackBufferWidth;
	float h = (float)mD3dpp.BackBufferHeight;
	D3DXMatrixPerspectiveFovLH(&mProj, D3DX_PI * 0.25f, w / h, 1.0f, 5000.0f);
}

void GateDemoApp::DrawGround()
{
	HR(mFX->SetValue(mhAmbientMtrl, &mGroundMtrl.ambient, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhDiffuseMtrl, &mGroundMtrl.diffuse, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhSpecularMtrl, &mGroundMtrl.spec, sizeof(D3DXCOLOR)));
	HR(mFX->SetFloat(mhSpecularPower, mGroundMtrl.specPower));

	HR(mFX->SetMatrix(mhWVP, &(mGroundWorld*mView*mProj)));
	D3DXMATRIX worldInvTrans;
	D3DXMatrixInverse(&worldInvTrans, 0, &mGroundWorld);
	D3DXMatrixTranspose(&worldInvTrans, &worldInvTrans);
	HR(mFX->SetMatrix(mhWorldInvTrans, &worldInvTrans));
	HR(mFX->SetMatrix(mhWorld, &mGroundWorld));
	HR(mFX->SetTexture(mhTex, mGroundTex));

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
}

void GateDemoApp::DrawGate()
{
	HR(gD3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, true));
	HR(gD3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL));
	HR(gD3dDevice->SetRenderState(D3DRS_ALPHAREF, 100));

	HR(gD3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));

	HR(mFX->SetValue(mhAmbientMtrl, &mGateMtrl.ambient, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhDiffuseMtrl, &mGateMtrl.diffuse, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhSpecularMtrl, &mGateMtrl.spec, sizeof(D3DXCOLOR)));
	HR(mFX->SetFloat(mhSpecularPower, mGateMtrl.specPower));

	HR(mFX->SetMatrix(mhWVP, &(mGateWorld*mView*mProj)));
	D3DXMATRIX worldInvTrans;
	D3DXMatrixInverse(&worldInvTrans, 0, &mGateWorld);
	D3DXMatrixTranspose(&worldInvTrans, &worldInvTrans);
	HR(mFX->SetMatrix(mhWorldInvTrans, &worldInvTrans));
	HR(mFX->SetMatrix(mhWorld, &mGateWorld));
	HR(mFX->SetTexture(mhTex, mGateTex));

	HR(gD3dDevice->SetVertexDeclaration(VertexPNT::decl));
	HR(gD3dDevice->SetStreamSource(0, mGateVB, 0, sizeof(VertexPNT)));
	HR(gD3dDevice->SetIndices(mGateIB));

	// Begin passes.
	UINT numPasses = 0;
	HR(mFX->Begin(&numPasses, 0));
	for (UINT i = 0; i < numPasses; ++i)
	{
		HR(mFX->BeginPass(i));
		HR(gD3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2));
		HR(mFX->EndPass());
	}
	HR(mFX->End());

	HR(gD3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, false));

	HR(gD3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW));
}
