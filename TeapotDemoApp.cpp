#include "Vertex.h"
#include "DirectInput.h"
#include "TeapotDemoApp.h"

TeapotDemoApp::TeapotDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP) : D3DApp(hInstance, winCaption, devType, requestedVP)
{
	if (!CheckDeviceCaps())
	{
		MessageBox(0, "checkDeviceCaps() Failed", 0, 0);
		PostQuitMessage(0);
	}

	InitAllVertexDeclarations();

	mGfxStats = new GfxStats();


	mCameraRadius = 6.0f;
	mCameraRotationY = 1.2 * D3DX_PI;
	mCameraHeight = 3.0f;

	mLightVecW = D3DXVECTOR3(0.0, 0.0f, -1.0f);
	mDiffuseLight = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	mAmbientLight = D3DXCOLOR(0.6f, 0.6f, 0.6f, 1.0f);
	mSpecularLight = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

	mCrateMtrl.ambient = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	mCrateMtrl.diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	mCrateMtrl.spec = D3DXCOLOR(0.4f, 0.4f, 0.4f, 1.0f);
	mCrateMtrl.specPower = 8.0f;

	mTeapotMtrl.ambient = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	mTeapotMtrl.diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 0.5f);
	mTeapotMtrl.spec = D3DXCOLOR(0.8f, 0.8f, 0.8f, 1.0f);
	mTeapotMtrl.specPower = 16.0f;

	D3DXMatrixTranslation(&mCrateWorld, 0.0f, 0.0f, 2.0f);
	D3DXMatrixIdentity(&mTeapotWorld);

	HR(D3DXCreateTextureFromFile(gD3dDevice, "crate.jpg", &mCrateTex));
	HR(D3DXCreateTextureFromFile(gD3dDevice, "bricka.dds", &mTeapotTex));

	HR(D3DXCreateTeapot(gD3dDevice, &mTeapot, 0));
	GenSphericalTexCoords();

	mGfxStats->AddVertices(24);
	mGfxStats->AddTriangles(12);
	mGfxStats->AddVertices(mTeapot->GetNumVertices());
	mGfxStats->AddTriangles(mTeapot->GetNumFaces());

	BuildBoxGeometry();
	BuildFX();

	OnResetDevice();
}

TeapotDemoApp::~TeapotDemoApp()
{
	delete mGfxStats;
	ReleaseCOM(mBoxVB);
	ReleaseCOM(mBoxIB);
	ReleaseCOM(mTeapot);
	ReleaseCOM(mCrateTex);
	ReleaseCOM(mTeapotTex);
	ReleaseCOM(mFX);

	DestroyAllVertexDeclarations();
}

bool TeapotDemoApp::CheckDeviceCaps()
{
	D3DCAPS9 caps;
	HR(gD3dDevice->GetDeviceCaps(&caps));

	if (caps.VertexShaderVersion < D3DVS_VERSION(2, 0))
		return false;

	if (caps.PixelShaderVersion < D3DPS_VERSION(2, 0))
		return false;

	return true;
}

void TeapotDemoApp::OnLostDevice()
{
	mGfxStats->OnLostDevice();
	HR(mFX->OnLostDevice());
}

void TeapotDemoApp::OnResetDevice()
{
	mGfxStats->OnResetDevice();
	HR(mFX->OnResetDevice());

	BuildProjMtx();
}

void TeapotDemoApp::UpdateScene(float dt)
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

	if (mCameraRadius < 3.0f)
		mCameraRadius = 3.0f;

	BuildViewMtx();
}

void TeapotDemoApp::DrawScene()
{
	HR(gD3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffeeeeee, 1.0f, 0));

	HR(gD3dDevice->BeginScene());

	HR(mFX->SetTechnique(mhTech));
	HR(mFX->SetValue(mhLightVecW, &mLightVecW, sizeof(D3DXVECTOR3)));
	HR(mFX->SetValue(mhDiffuseLight, &mDiffuseLight, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhAmbientLight, &mAmbientLight, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhSpecularLight, &mSpecularLight, sizeof(D3DXCOLOR)));

	DrawCrate();
	DrawTeapot();

	mGfxStats->Display();

	HR(gD3dDevice->EndScene());

	HR(gD3dDevice->Present(0, 0, 0, 0));
}

void TeapotDemoApp::BuildBoxGeometry()
{
	HR(gD3dDevice->CreateVertexBuffer(24 * sizeof(VertexPNT), D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &mBoxVB, 0));

	VertexPNT* v = 0;
	HR(mBoxVB->Lock(0, 0, (void**)&v, 0));

	// Fill in the front face vertex data.
	v[0] = VertexPNT(-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[1] = VertexPNT(-1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[2] = VertexPNT(1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	v[3] = VertexPNT(1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

	// Fill in the back face vertex data.
	v[4] = VertexPNT(-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	v[5] = VertexPNT(1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	v[6] = VertexPNT(1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
	v[7] = VertexPNT(-1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);

	// Fill in the top face vertex data.
	v[8] = VertexPNT(-1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
	v[9] = VertexPNT(-1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);
	v[10] = VertexPNT(1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f);
	v[11] = VertexPNT(1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f);

	// Fill in the bottom face vertex data.
	v[12] = VertexPNT(-1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f);
	v[13] = VertexPNT(1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f);
	v[14] = VertexPNT(1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f);
	v[15] = VertexPNT(-1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f);

	// Fill in the left face vertex data.
	v[16] = VertexPNT(-1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[17] = VertexPNT(-1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[18] = VertexPNT(-1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[19] = VertexPNT(-1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	// Fill in the right face vertex data.
	v[20] = VertexPNT(1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[21] = VertexPNT(1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[22] = VertexPNT(1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[23] = VertexPNT(1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	HR(mBoxVB->Unlock());

	HR(gD3dDevice->CreateIndexBuffer(36 * sizeof(WORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &mBoxIB, 0));

	WORD* i = 0;
	HR(mBoxIB->Lock(0, 0, (void**)&i, 0));

	// Fill in the front face index data
	i[0] = 0; i[1] = 1; i[2] = 2;
	i[3] = 0; i[4] = 2; i[5] = 3;

	// Fill in the back face index data
	i[6] = 4; i[7] = 5; i[8] = 6;
	i[9] = 4; i[10] = 6; i[11] = 7;

	// Fill in the top face index data
	i[12] = 8; i[13] = 9; i[14] = 10;
	i[15] = 8; i[16] = 10; i[17] = 11;

	// Fill in the bottom face index data
	i[18] = 12; i[19] = 13; i[20] = 14;
	i[21] = 12; i[22] = 14; i[23] = 15;

	// Fill in the left face index data
	i[24] = 16; i[25] = 17; i[26] = 18;
	i[27] = 16; i[28] = 18; i[29] = 19;

	// Fill in the right face index data
	i[30] = 20; i[31] = 21; i[32] = 22;
	i[33] = 20; i[34] = 22; i[35] = 23;

	HR(mBoxIB->Unlock());
}

void TeapotDemoApp::BuildFX()
{
	ID3DXBuffer* errors = NULL;
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

void TeapotDemoApp::BuildViewMtx()
{
	float x = mCameraRadius * cosf(mCameraRotationY);
	float z = mCameraRadius * sinf(mCameraRotationY);
	D3DXVECTOR3 pos(x, mCameraHeight, z);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&mView, &pos, &target, &up);

	HR(mFX->SetValue(mhEyePos, &pos, sizeof(D3DXVECTOR3)));
}

void TeapotDemoApp::BuildProjMtx()
{
	float w = (float)mD3dpp.BackBufferWidth;
	float h = (float)mD3dpp.BackBufferHeight;
	D3DXMatrixPerspectiveFovLH(&mProj, D3DX_PI * 0.25f, w / h, 1.0f, 5000.0f);
}

void TeapotDemoApp::DrawCrate()
{
	HR(mFX->SetValue(mhAmbientMtrl, &mCrateMtrl.ambient, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhDiffuseMtrl, &mCrateMtrl.diffuse, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhSpecularMtrl, &mCrateMtrl.spec, sizeof(D3DXCOLOR)));
	HR(mFX->SetFloat(mhSpecularPower, mCrateMtrl.specPower));

	HR(mFX->SetMatrix(mhWVP, &(mCrateWorld*mView*mProj)));
	D3DXMATRIX worldInvTrans;
	D3DXMatrixInverse(&worldInvTrans, 0, &mCrateWorld);
	D3DXMatrixTranspose(&worldInvTrans, &worldInvTrans);
	HR(mFX->SetMatrix(mhWorldInvTrans, &worldInvTrans));
	HR(mFX->SetMatrix(mhWorld, &mCrateWorld));
	HR(mFX->SetTexture(mhTex, mCrateTex));

	HR(gD3dDevice->SetVertexDeclaration(VertexPNT::decl));
	HR(gD3dDevice->SetStreamSource(0, mBoxVB, 0, sizeof(VertexPNT)));
	HR(gD3dDevice->SetIndices(mBoxIB));

	UINT numPasses = 0;
	HR(mFX->Begin(&numPasses, 0));
	for (UINT i = 0; i < numPasses; ++i)
	{
		HR(mFX->BeginPass(i));
		HR(gD3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 24, 0, 12));
		HR(mFX->EndPass());
	}
	HR(mFX->End());
}

void TeapotDemoApp::DrawTeapot()
{
	HR(gD3dDevice->SetRenderState(D3DRS_WRAP0, D3DWRAPCOORD_0));

	HR(gD3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true));
	HR(gD3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
	HR(gD3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));

	HR(mFX->SetValue(mhAmbientMtrl, &mTeapotMtrl.ambient, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhDiffuseMtrl, &mTeapotMtrl.diffuse, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhSpecularMtrl, &mTeapotMtrl.spec, sizeof(D3DXCOLOR)));
	HR(mFX->SetFloat(mhSpecularPower, mTeapotMtrl.specPower));

	HR(mFX->SetMatrix(mhWVP, &(mTeapotWorld*mView*mProj)));
	D3DXMATRIX worldInvTrans;
	D3DXMatrixInverse(&worldInvTrans, 0, &mTeapotWorld);
	D3DXMatrixTranspose(&worldInvTrans, &worldInvTrans);
	HR(mFX->SetMatrix(mhWorldInvTrans, &worldInvTrans));
	HR(mFX->SetMatrix(mhWorld, &mTeapotWorld));
	HR(mFX->SetTexture(mhTex, mTeapotTex));

	// Begin passes.
	UINT numPasses = 0;
	HR(mFX->Begin(&numPasses, 0));
	for (UINT i = 0; i < numPasses; ++i)
	{
		HR(mFX->BeginPass(i));
		HR(mTeapot->DrawSubset(0));
		HR(mFX->EndPass());
	}
	HR(mFX->End());

	HR(gD3dDevice->SetRenderState(D3DRS_WRAP0, 0));

	HR(gD3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false));
}

void TeapotDemoApp::GenSphericalTexCoords()
{
	D3DVERTEXELEMENT9 elements[64];
	UINT numElements = 0;
	VertexPNT::decl->GetDeclaration(elements, &numElements);

	ID3DXMesh* temp = NULL;
	HR(mTeapot->CloneMesh(D3DXMESH_SYSTEMMEM, elements, gD3dDevice, &temp));

	ReleaseCOM(mTeapot);

	VertexPNT* vertices = NULL;
	HR(temp->LockVertexBuffer(0, (void**)&vertices));

	for (UINT i = 0; i < temp->GetNumVertices(); ++i)
	{
		D3DXVECTOR3 p = vertices[i].pos;

		float theta = atan2f(p.z, p.x);
		float phi = acosf(p.y / sqrtf(p.x*p.x + p.y*p.y + p.z*p.z));

		float u = theta / (2.0f * D3DX_PI);
		float v = phi / D3DX_PI;

		vertices[i].tex0.x = u;
		vertices[i].tex0.y = v;
	}
	HR(temp->UnlockVertexBuffer());

	HR(temp->CloneMesh(D3DXMESH_MANAGED | D3DXMESH_WRITEONLY, elements, gD3dDevice, &mTeapot));

	ReleaseCOM(temp);
}
