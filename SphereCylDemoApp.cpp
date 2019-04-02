#include "Vertex.h"
#include "DirectInput.h"
#include "SphereCylDemoApp.h"

SphereCylDemoApp::SphereCylDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP) : D3DApp(hInstance, winCaption, devType, requestedVP)
{
	if (!CheckDeviceCaps())
	{
		MessageBox(0, "checkDeviceCaps() Failed", 0, 0);
		PostQuitMessage(0);
	}

	InitAllVertexDeclarations();

	mGfxStats = new GfxStats();

	mCameraRadius = 50.0f;
	mCameraRotationY = 1.2 * D3DX_PI;
	mCameraHeight = 20.0f;

	mAmbientLight = _WHITE;
	mDiffuseLight = _WHITE;
	mSpecLight = _WHITE;
	mLightVecW = D3DXVECTOR3(0.0, 0.0f, -1.0f);

	mGridMtrl = Mtrl(_WHITE*0.7f, _WHITE, _WHITE*0.5f, 16.0f);
	mCylinderMtrl = Mtrl(_WHITE*0.4f, _WHITE, _WHITE*0.8f, 8.0f);
	mSphereMtrl = Mtrl(_WHITE*0.4f, _WHITE, _WHITE*0.8f, 8.0f);

	HR(D3DXCreateCylinder(gD3dDevice, 1.0f, 1.0f, 6.0f, 20, 20, &mCylinder, 0));
	HR(D3DXCreateSphere(gD3dDevice, 1.0f, 20, 20, &mSphere, 0));

	GenSphericalTexCoords();
	GenCylTexCoords(Z_AXIS);

	HR(D3DXCreateTextureFromFile(gD3dDevice, "marble.bmp", &mSphereTex));
	HR(D3DXCreateTextureFromFile(gD3dDevice, "stone2.dds", &mCylTex));
	HR(D3DXCreateTextureFromFile(gD3dDevice, "ground0.dds", &mGridTex));

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
}

SphereCylDemoApp::~SphereCylDemoApp()
{
	delete mGfxStats;
	ReleaseCOM(mVB);
	ReleaseCOM(mIB);
	ReleaseCOM(mFX);
	ReleaseCOM(mCylinder);
	ReleaseCOM(mSphere);
	ReleaseCOM(mSphereTex);
	ReleaseCOM(mCylTex);
	ReleaseCOM(mGridTex);

	DestroyAllVertexDeclarations();
}

bool SphereCylDemoApp::CheckDeviceCaps()
{
	D3DCAPS9 caps;
	HR(gD3dDevice->GetDeviceCaps(&caps));

	if (caps.VertexShaderVersion < D3DVS_VERSION(2, 0))
		return false;

	if (caps.PixelShaderVersion < D3DPS_VERSION(2, 0))
		return false;

	return true;
}

void SphereCylDemoApp::OnLostDevice()
{
	mGfxStats->OnLostDevice();
	HR(mFX->OnLostDevice());
}

void SphereCylDemoApp::OnResetDevice()
{
	mGfxStats->OnResetDevice();
	HR(mFX->OnResetDevice());

	BuildProjMtx();
}

void SphereCylDemoApp::UpdateScene(float dt)
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

void SphereCylDemoApp::DrawScene()
{
	HR(gD3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0));

	HR(gD3dDevice->BeginScene());

	HR(mFX->SetValue(mhAmbientLight, &mAmbientLight, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhDiffuseLight, &mDiffuseLight, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhSpecLight, &mSpecLight, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhLightVecW, &mLightVecW, sizeof(D3DXVECTOR3)));

	UINT numPasses = 0;
	HR(mFX->Begin(&numPasses, 0));
	for (UINT i = 0; i < numPasses; ++i)
	{
		HR(mFX->BeginPass(i));

		DrawGrid();
		DrawCylinders();
		DrawSpheres();

		HR(mFX->EndPass());
	}
	HR(mFX->End());


	mGfxStats->Display();

	HR(gD3dDevice->EndScene());

	HR(gD3dDevice->Present(0, 0, 0, 0));
}

void SphereCylDemoApp::BuildGeoBuffers()
{
	std::vector<D3DXVECTOR3> verts;
	std::vector<DWORD> indices;

	GenTriGrid(100, 100, 1.0f, 1.0f, D3DXVECTOR3(0.0f, 0.0f, 0.0f), verts, indices);

	mNumGridVertices = 100 * 100;
	mNumGridTriangles = 99 * 99 * 2;

	HR(gD3dDevice->CreateVertexBuffer(mNumGridVertices * sizeof(VertexPNT), D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &mVB, 0));

	VertexPNT* v = 0;
	HR(mVB->Lock(0, 0, (void**)&v, 0));

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

	HR(mVB->Unlock());

	HR(gD3dDevice->CreateIndexBuffer(mNumGridTriangles * 3 * sizeof(WORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &mIB, 0));

	WORD* k = 0;
	HR(mIB->Lock(0, 0, (void**)&k, 0));

	for (DWORD i = 0; i < mNumGridTriangles * 3; ++i)
		k[i] = (WORD)indices[i];

	HR(mIB->Unlock());
}

void SphereCylDemoApp::BuildFx()
{
	ID3DXBuffer* errors = 0;
	HR(D3DXCreateEffectFromFile(gD3dDevice, "dirLightTex.fx", 0, 0, D3DXSHADER_DEBUG, 0, &mFX, &errors));
	if (errors)
		MessageBox(0, (char*)errors->GetBufferPointer(), 0, 0);

	mhTech = mFX->GetTechniqueByName("DirLightTexTech");
	mhWVP = mFX->GetParameterByName(0, "gWVP");
	mhWorldInvTrans = mFX->GetParameterByName(0, "gWorldInvTrans");
	mhEyePos = mFX->GetParameterByName(0, "gEyePosW");
	mhWorld = mFX->GetParameterByName(0, "gWorld");
	mhAmbientLight = mFX->GetParameterByName(0, "gAmbientLight");
	mhDiffuseLight = mFX->GetParameterByName(0, "gDiffuseLight");
	mhSpecLight = mFX->GetParameterByName(0, "gSpecularLight");
	mhLightVecW = mFX->GetParameterByName(0, "gLightVecW");
	mhAmbientMtrl = mFX->GetParameterByName(0, "gAmbientMtrl");
	mhDiffuseMtrl = mFX->GetParameterByName(0, "gDiffuseMtrl");
	mhSpecMtrl = mFX->GetParameterByName(0, "gSpecularMtrl");
	mhSpecPower = mFX->GetParameterByName(0, "gSpecularPower");
	mhTex = mFX->GetParameterByName(0, "gTex");
}

void SphereCylDemoApp::BuildViewMtx()
{
	float x = mCameraRadius * cosf(mCameraRotationY);
	float z = mCameraRadius * sinf(mCameraRotationY);
	D3DXVECTOR3 pos(x, mCameraHeight, z);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&mView, &pos, &target, &up);

	HR(mFX->SetValue(mhEyePos, &pos, sizeof(D3DXVECTOR3)));
}

void SphereCylDemoApp::BuildProjMtx()
{
	float w = (float)mD3dpp.BackBufferWidth;
	float h = (float)mD3dpp.BackBufferHeight;
	D3DXMatrixPerspectiveFovLH(&mProj, D3DX_PI * 0.25f, w / h, 1.0f, 5000.0f);
}

void SphereCylDemoApp::DrawGrid()
{
	HR(gD3dDevice->SetStreamSource(0, mVB, 0, sizeof(VertexPNT)));
	HR(gD3dDevice->SetIndices(mIB));
	HR(gD3dDevice->SetVertexDeclaration(VertexPNT::decl));

	D3DXMATRIX W, WIT;
	D3DXMatrixIdentity(&W);
	D3DXMatrixInverse(&WIT, 0, &W);
	D3DXMatrixTranspose(&WIT, &WIT);
	HR(mFX->SetMatrix(mhWorld, &W));
	HR(mFX->SetMatrix(mhWVP, &(W*mView*mProj)));
	HR(mFX->SetMatrix(mhWorldInvTrans, &WIT));

	HR(mFX->SetValue(mhAmbientMtrl, &mGridMtrl.ambient, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhDiffuseMtrl, &mGridMtrl.diffuse, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhSpecMtrl, &mGridMtrl.spec, sizeof(D3DXCOLOR)));
	HR(mFX->SetFloat(mhSpecPower, mGridMtrl.specPower));

	HR(mFX->SetTexture(mhTex, mGridTex));

	HR(mFX->CommitChanges());
	HR(gD3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, mNumGridVertices, 0, mNumGridTriangles));
}

void SphereCylDemoApp::DrawCylinders()
{
	HR(gD3dDevice->SetRenderState(D3DRS_WRAP0, D3DWRAP_U));

	D3DXMATRIX T, R, W, WIT;

	D3DXMatrixRotationX(&R, -D3DX_PI * 0.5f);

	HR(mFX->SetValue(mhAmbientMtrl, &mCylinderMtrl.ambient, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhDiffuseMtrl, &mCylinderMtrl.diffuse, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhSpecMtrl, &mCylinderMtrl.spec, sizeof(D3DXCOLOR)));
	HR(mFX->SetFloat(mhSpecPower, mCylinderMtrl.specPower));

	HR(mFX->SetTexture(mhTex, mCylTex));
	for (int z = -30; z <= 30; z += 10)
	{
		D3DXMatrixTranslation(&T, -10.0f, 3.0f, (float)z);
		W = R * T;
		D3DXMatrixInverse(&WIT, 0, &W);
		D3DXMatrixTranspose(&WIT, &WIT);

		HR(mFX->SetMatrix(mhWVP, &(W*mView*mProj)));
		HR(mFX->SetMatrix(mhWorld, &W));
		HR(mFX->SetMatrix(mhWorldInvTrans, &WIT));
		HR(mFX->CommitChanges());
		HR(mCylinder->DrawSubset(0));

		D3DXMatrixTranslation(&T, 10.0f, 3.0f, (float)z);
		W = R * T;
		D3DXMatrixInverse(&WIT, 0, &W);
		D3DXMatrixTranspose(&WIT, &WIT);

		HR(mFX->SetMatrix(mhWVP, &(W*mView*mProj)));
		HR(mFX->SetMatrix(mhWorld, &W));
		HR(mFX->SetMatrix(mhWorldInvTrans, &WIT));
		HR(mFX->CommitChanges());
		HR(mCylinder->DrawSubset(0));
	}
	HR(gD3dDevice->SetRenderState(D3DRS_WRAP0, 0));
}

void SphereCylDemoApp::DrawSpheres()
{
	HR(gD3dDevice->SetRenderState(D3DRS_WRAP0, D3DWRAP_U));

	D3DXMATRIX W, WIT;

	HR(mFX->SetValue(mhAmbientMtrl, &mSphereMtrl.ambient, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhDiffuseMtrl, &mSphereMtrl.diffuse, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhSpecMtrl, &mSphereMtrl.spec, sizeof(D3DXCOLOR)));
	HR(mFX->SetFloat(mhSpecPower, mSphereMtrl.specPower));

	HR(mFX->SetTexture(mhTex, mSphereTex));
	for (int z = -30; z <= 30; z += 10)
	{
		D3DXMatrixTranslation(&W, -10.0f, 7.5f, (float)z);
		D3DXMatrixInverse(&WIT, 0, &W);
		D3DXMatrixTranspose(&WIT, &WIT);

		HR(mFX->SetMatrix(mhWVP, &(W*mView*mProj)));
		HR(mFX->SetMatrix(mhWorld, &W));
		HR(mFX->SetMatrix(mhWorldInvTrans, &WIT));
		HR(mFX->CommitChanges());
		HR(mSphere->DrawSubset(0));

		D3DXMatrixTranslation(&W, 10.0f, 7.5f, (float)z);
		D3DXMatrixInverse(&WIT, 0, &W);
		D3DXMatrixTranspose(&WIT, &WIT);

		HR(mFX->SetMatrix(mhWVP, &(W*mView*mProj)));
		HR(mFX->SetMatrix(mhWorld, &W));
		HR(mFX->SetMatrix(mhWorldInvTrans, &WIT));
		HR(mFX->CommitChanges());
		HR(mSphere->DrawSubset(0));
	}
	HR(gD3dDevice->SetRenderState(D3DRS_WRAP0, 0));
}

void SphereCylDemoApp::GenSphericalTexCoords()
{
	D3DVERTEXELEMENT9 elements[64];
	UINT numElements = 0;
	VertexPNT::decl->GetDeclaration(elements, &numElements);

	ID3DXMesh *temp = NULL;
	HR(mSphere->CloneMesh(D3DXMESH_SYSTEMMEM, elements, gD3dDevice, &temp));
	ReleaseCOM(mSphere);

	VertexPNT *vertices = NULL;
	HR(temp->LockVertexBuffer(0, (void **)&vertices));

	for (UINT i = 0; i < temp->GetNumVertices(); ++i)
	{
		D3DXVECTOR3 p = vertices[i].pos;
		float theta = atan2f(p.z, p.x);
		float phi = acosf(p.y / sqrtf(p.x * p.x + p.y * p.y + p.z * p.z));

		float u = theta / (2.0f * D3DX_PI);
		float v = phi / D3DX_PI;

		vertices[i].tex0.x = u;
		vertices[i].tex0.y = v;
	}

	HR(temp->UnlockVertexBuffer());
	HR(temp->CloneMesh(D3DXMESH_MANAGED | D3DXMESH_WRITEONLY, elements, gD3dDevice, &mSphere));
	ReleaseCOM(temp);
}

void SphereCylDemoApp::GenCylTexCoords(AXIS axis)
{
	D3DVERTEXELEMENT9 elements[64];
	UINT numElements = 0;
	VertexPNT::decl->GetDeclaration(elements, &numElements);

	ID3DXMesh *temp = NULL;
	HR(mCylinder->CloneMesh(D3DXMESH_SYSTEMMEM, elements, gD3dDevice, &temp));
	ReleaseCOM(mCylinder);

	VertexPNT *vertices = NULL;
	HR(temp->LockVertexBuffer(0, (void **)&vertices));

	D3DXVECTOR3 maxPoint(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	D3DXVECTOR3 minPoint(FLT_MAX, FLT_MAX, FLT_MAX);

	for (UINT i = 0; i < temp->GetNumVertices(); ++i)
	{
		D3DXVec3Maximize(&maxPoint, &maxPoint, &vertices[i].pos);
		D3DXVec3Minimize(&minPoint, &minPoint, &vertices[i].pos);
	}

	float a = 0.0f;
	float b = 0.0f;
	float h = 0.0f;
	switch (axis)
	{
	case X_AXIS:
		a = minPoint.x;
		b = maxPoint.x;
		h = b - a;
		break;
	case Y_AXIS:
		a = minPoint.y;
		b = maxPoint.y;
		h = b - a;
		break;
	case Z_AXIS:
		a = minPoint.z;
		b = maxPoint.z;
		h = b - a;
		break;
	default:
		break;
	}

	for (UINT i = 0; i < temp->GetNumVertices(); ++i)
	{
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
		switch (axis)
		{
		case X_AXIS:
			x = vertices[i].pos.y;
			z = vertices[i].pos.z;
			y = vertices[i].pos.x;
			break;
		case Y_AXIS:
			x = vertices[i].pos.x;
			z = vertices[i].pos.z;
			y = vertices[i].pos.y;
			break;
		case Z_AXIS:
			x = vertices[i].pos.x;
			z = vertices[i].pos.y;
			y = vertices[i].pos.z;
			break;
		default:
			break;
		}

		float theta = atan2f(z, x);
		float y2 = y - b;

		float u = theta / (2.0f * D3DX_PI);
		float v = y2 / -h;

		vertices[i].tex0.x = u;
		vertices[i].tex0.y = v;
	}

	HR(temp->UnlockVertexBuffer());
	HR(temp->CloneMesh(D3DXMESH_MANAGED | D3DXMESH_WRITEONLY, elements, gD3dDevice, &mCylinder));
	ReleaseCOM(temp);
}
