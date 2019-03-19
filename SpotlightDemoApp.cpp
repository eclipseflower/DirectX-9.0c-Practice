#include "SpotlightDemoApp.h"
#include "DirectInput.h"
#include "Vertex.h"

SpotlightDemoApp::SpotlightDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP) : D3DApp(hInstance, winCaption, devType, requestedVP)
{
	if (!CheckDeviceCaps())
	{
		MessageBox(0, "checkDeviceCaps() Failed", 0, 0);
		PostQuitMessage(0);
	}

	mGfxStats = new GfxStats();

	mCameraRadius = 50.0f;
	mCameraRotationY = 1.2 * D3DX_PI;
	mCameraHeight = 20.0f;

	mAmbientLight = 0.4f * _WHITE;
	mDiffuseLight = _WHITE;
	mSpecLight = _WHITE;
	mAttenuation012 = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
	mSpotPower = 16.0f;

	mGridMtrl = Mtrl(_BLUE, _BLUE, _WHITE, 16.0f);
	mCylinderMtrl = Mtrl(_RED, _RED, _WHITE, 8.0f);
	mSphereMtrl = Mtrl(_GREEN, _GREEN, _WHITE, 8.0f);

	HR(D3DXCreateCylinder(gD3dDevice, 1.0f, 1.0f, 6.0f, 20, 20, &mCylinder, 0));
	HR(D3DXCreateSphere(gD3dDevice, 1.0f, 20, 20, &mSphere, 0));

	BuildGeoBuffers();
	BuildFx();

	int numCylVerts = mCylinder->GetNumVertices() * 14;
	int numSphereVerts = mSphere->GetNumVertices() * 14;
	int numCylTris = mCylinder->GetNumFaces() * 14;
	int numSphereTris = mSphere->GetNumFaces() * 14;

	mGfxStats->AddVertices(mNumGridVertices);
	mGfxStats->AddVertices(numCylVerts);
	mGfxStats->AddVertices(numSphereVerts);
	mGfxStats->AddTriangles(mNumGridTriangles);
	mGfxStats->AddTriangles(numCylTris);
	mGfxStats->AddTriangles(numSphereTris);

	OnResetDevice();

	InitAllVertexDeclarations();
}

SpotlightDemoApp::~SpotlightDemoApp()
{
	delete mGfxStats;
	ReleaseCOM(mVb);
	ReleaseCOM(mIb);
	ReleaseCOM(mFx);
	ReleaseCOM(mCylinder);
	ReleaseCOM(mSphere);

	DestroyAllVertexDeclarations();
}

bool SpotlightDemoApp::CheckDeviceCaps()
{
	D3DCAPS9 caps;
	HR(gD3dDevice->GetDeviceCaps(&caps));

	if (caps.VertexShaderVersion < D3DVS_VERSION(2, 0))
		return false;

	if (caps.PixelShaderVersion < D3DPS_VERSION(2, 0))
		return false;

	return true;
}

void SpotlightDemoApp::OnLostDevice()
{
	mGfxStats->OnLostDevice();
	HR(mFx->OnLostDevice());
}

void SpotlightDemoApp::OnResetDevice()
{
	mGfxStats->OnResetDevice();
	HR(mFx->OnResetDevice());

	BuildProjMtx();
}

void SpotlightDemoApp::UpdateScene(float dt)
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

	if (gDInput->keyDown(DIK_G))
		mSpotPower += 25.0f * dt;
	if (gDInput->keyDown(DIK_H))
		mSpotPower -= 25.0f * dt;

	if (mSpotPower < 1.0f)
		mSpotPower = 1.0f;
	if (mSpotPower > 64.0f)
		mSpotPower = 64.0f;

	BuildViewMtx();
}

void SpotlightDemoApp::DrawScene()
{
	HR(gD3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0));

	HR(gD3dDevice->BeginScene());

	HR(mFx->SetValue(mhAmbientLight, &mAmbientLight, sizeof(D3DXCOLOR)));
	HR(mFx->SetValue(mhDiffuseLight, &mDiffuseLight, sizeof(D3DXCOLOR)));
	HR(mFx->SetValue(mhSpecLight, &mSpecLight, sizeof(D3DXCOLOR)));
	HR(mFx->SetValue(mhAttenuation012, &mAttenuation012, sizeof(D3DXVECTOR3)));
	HR(mFx->SetFloat(mhSpotPower, mSpotPower));

	UINT numPasses = 0;
	HR(mFx->Begin(&numPasses, 0));
	for (UINT i = 0; i < numPasses; ++i)
	{
		HR(mFx->BeginPass(i));

		DrawGrid();
		DrawCylinders();
		DrawSpheres();

		HR(mFx->EndPass());
	}
	HR(mFx->End());


	mGfxStats->Display();

	HR(gD3dDevice->EndScene());

	HR(gD3dDevice->Present(0, 0, 0, 0));
}

void SpotlightDemoApp::BuildGeoBuffers()
{
	std::vector<D3DXVECTOR3> verts;
	std::vector<DWORD> indices;

	GenTriGrid(100, 100, 1.0f, 1.0f, D3DXVECTOR3(0.0f, 0.0f, 0.0f), verts, indices);

	mNumGridVertices = 100 * 100;
	mNumGridTriangles = 99 * 99 * 2;

	HR(gD3dDevice->CreateVertexBuffer(mNumGridVertices * sizeof(VertexPN), D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &mVb, 0));

	VertexPN* v = NULL;
	HR(mVb->Lock(0, 0, (void**)&v, 0));

	for (DWORD i = 0; i < mNumGridVertices; ++i)
	{
		v[i].pos = verts[i];
		v[i].normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	}

	HR(mVb->Unlock());

	HR(gD3dDevice->CreateIndexBuffer(mNumGridTriangles * 3 * sizeof(WORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &mIb, 0));

	WORD* k = NULL;
	HR(mIb->Lock(0, 0, (void**)&k, 0));

	for (DWORD i = 0; i < mNumGridTriangles * 3; ++i)
		k[i] = (WORD)indices[i];

	HR(mIb->Unlock());
}

void SpotlightDemoApp::BuildFx()
{
	ID3DXBuffer* errors = NULL;
	HR(D3DXCreateEffectFromFile(gD3dDevice, "spotlight.fx", 0, 0, D3DXSHADER_DEBUG, 0, &mFx, &errors));
	if (errors)
		MessageBox(0, (char*)errors->GetBufferPointer(), 0, 0);

	mhTech = mFx->GetTechniqueByName("SpotlightTech");
	mhWVP = mFx->GetParameterByName(0, "gWVP");
	mhWorldInvTrans = mFx->GetParameterByName(0, "gWorldInvTrans");
	mhEyePos = mFx->GetParameterByName(0, "gEyePosW");
	mhWorld = mFx->GetParameterByName(0, "gWorld");
	mhAmbientLight = mFx->GetParameterByName(0, "gAmbientLight");
	mhDiffuseLight = mFx->GetParameterByName(0, "gDiffuseLight");
	mhSpecLight = mFx->GetParameterByName(0, "gSpecLight");
	mhLightPosW = mFx->GetParameterByName(0, "gLightPosW");
	mhLightDirW = mFx->GetParameterByName(0, "gLightDirW");
	mhAttenuation012 = mFx->GetParameterByName(0, "gAttenuation012");
	mhAmbientMtrl = mFx->GetParameterByName(0, "gAmbientMtrl");
	mhDiffuseMtrl = mFx->GetParameterByName(0, "gDiffuseMtrl");
	mhSpecMtrl = mFx->GetParameterByName(0, "gSpecMtrl");
	mhSpecPower = mFx->GetParameterByName(0, "gSpecPower");
	mhSpotPower = mFx->GetParameterByName(0, "gSpotPower");
}

void SpotlightDemoApp::BuildViewMtx()
{
	float x = mCameraRadius * cosf(mCameraRotationY);
	float z = mCameraRadius * sinf(mCameraRotationY);
	D3DXVECTOR3 pos(x, mCameraHeight, z);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&mView, &pos, &target, &up);

	HR(mFx->SetValue(mhEyePos, &pos, sizeof(D3DXVECTOR3)));

	HR(mFx->SetValue(mhLightPosW, &pos, sizeof(D3DXVECTOR3)));

	D3DXVECTOR3 lightDir = target - pos;
	D3DXVec3Normalize(&lightDir, &lightDir);
	HR(mFx->SetValue(mhLightDirW, &lightDir, sizeof(D3DXVECTOR3)));
}

void SpotlightDemoApp::BuildProjMtx()
{
	float w = (float)mD3dpp.BackBufferWidth;
	float h = (float)mD3dpp.BackBufferHeight;
	D3DXMatrixPerspectiveFovLH(&mProj, D3DX_PI * 0.25f, w / h, 1.0f, 5000.0f);
}

void SpotlightDemoApp::DrawGrid()
{
	HR(gD3dDevice->SetStreamSource(0, mVb, 0, sizeof(VertexPN)));
	HR(gD3dDevice->SetIndices(mIb));
	HR(gD3dDevice->SetVertexDeclaration(VertexPN::decl));

	D3DXMATRIX W, WIT;
	D3DXMatrixIdentity(&W);
	D3DXMatrixInverse(&WIT, 0, &W);
	D3DXMatrixTranspose(&WIT, &WIT);
	HR(mFx->SetMatrix(mhWorld, &W));
	HR(mFx->SetMatrix(mhWVP, &(W*mView*mProj)));
	HR(mFx->SetMatrix(mhWorldInvTrans, &WIT));

	HR(mFx->SetValue(mhAmbientMtrl, &mGridMtrl.ambient, sizeof(D3DXCOLOR)));
	HR(mFx->SetValue(mhDiffuseMtrl, &mGridMtrl.diffuse, sizeof(D3DXCOLOR)));
	HR(mFx->SetValue(mhSpecMtrl, &mGridMtrl.spec, sizeof(D3DXCOLOR)));
	HR(mFx->SetFloat(mhSpecPower, mGridMtrl.specPower));

	HR(mFx->CommitChanges());
	HR(gD3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, mNumGridVertices, 0, mNumGridTriangles));
}

void SpotlightDemoApp::DrawCylinders()
{
	D3DXMATRIX T, R, W, WIT;

	D3DXMatrixRotationX(&R, D3DX_PI*0.5f);

	HR(mFx->SetValue(mhAmbientMtrl, &mCylinderMtrl.ambient, sizeof(D3DXCOLOR)));
	HR(mFx->SetValue(mhDiffuseMtrl, &mCylinderMtrl.diffuse, sizeof(D3DXCOLOR)));
	HR(mFx->SetValue(mhSpecMtrl, &mCylinderMtrl.spec, sizeof(D3DXCOLOR)));
	HR(mFx->SetFloat(mhSpecPower, mCylinderMtrl.specPower));
	for (int z = -30; z <= 30; z += 10)
	{
		D3DXMatrixTranslation(&T, -10.0f, 3.0f, (float)z);
		W = R * T;
		D3DXMatrixInverse(&WIT, 0, &W);
		D3DXMatrixTranspose(&WIT, &WIT);

		HR(mFx->SetMatrix(mhWVP, &(W*mView*mProj)));
		HR(mFx->SetMatrix(mhWorld, &W));
		HR(mFx->SetMatrix(mhWorldInvTrans, &WIT));
		HR(mFx->CommitChanges());
		HR(mCylinder->DrawSubset(0));

		D3DXMatrixTranslation(&T, 10.0f, 3.0f, (float)z);
		W = R * T;
		D3DXMatrixInverse(&WIT, 0, &W);
		D3DXMatrixTranspose(&WIT, &WIT);

		HR(mFx->SetMatrix(mhWVP, &(W*mView*mProj)));
		HR(mFx->SetMatrix(mhWorld, &W));
		HR(mFx->SetMatrix(mhWorldInvTrans, &WIT));
		HR(mFx->CommitChanges());
		HR(mCylinder->DrawSubset(0));
	}
}

void SpotlightDemoApp::DrawSpheres()
{
	D3DXMATRIX W, WIT;

	HR(mFx->SetValue(mhAmbientMtrl, &mSphereMtrl.ambient, sizeof(D3DXCOLOR)));
	HR(mFx->SetValue(mhDiffuseMtrl, &mSphereMtrl.diffuse, sizeof(D3DXCOLOR)));
	HR(mFx->SetValue(mhSpecMtrl, &mSphereMtrl.spec, sizeof(D3DXCOLOR)));
	HR(mFx->SetFloat(mhSpecPower, mSphereMtrl.specPower));
	for (int z = -30; z <= 30; z += 10)
	{
		D3DXMatrixTranslation(&W, -10.0f, 7.5f, (float)z);
		D3DXMatrixInverse(&WIT, 0, &W);
		D3DXMatrixTranspose(&WIT, &WIT);

		HR(mFx->SetMatrix(mhWVP, &(W*mView*mProj)));
		HR(mFx->SetMatrix(mhWorld, &W));
		HR(mFx->SetMatrix(mhWorldInvTrans, &WIT));
		HR(mFx->CommitChanges());
		HR(mSphere->DrawSubset(0));

		D3DXMatrixTranslation(&W, 10.0f, 7.5f, (float)z);
		D3DXMatrixInverse(&WIT, 0, &W);
		D3DXMatrixTranspose(&WIT, &WIT);

		HR(mFx->SetMatrix(mhWVP, &(W*mView*mProj)));
		HR(mFx->SetMatrix(mhWorld, &W));
		HR(mFx->SetMatrix(mhWorldInvTrans, &WIT));
		HR(mFx->CommitChanges());
		HR(mSphere->DrawSubset(0));
	}
}
