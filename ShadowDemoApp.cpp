#include "DirectInput.h"
#include "Vertex.h"
#include "ShadowDemoApp.h"

ShadowDemoApp::ShadowDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP) : D3DApp(hInstance, winCaption, devType, requestedVP)
{
	if (!CheckDeviceCaps())
	{
		MessageBox(0, "checkDeviceCaps() Failed", 0, 0);
		PostQuitMessage(0);
	}

	InitAllVertexDeclarations();

	mGfxStats = new GfxStats();


	mCameraRadius = 15.0f;
	mCameraRotationY = 1.4f * D3DX_PI;
	mCameraHeight = 5.0f;

	mLightVecW = D3DXVECTOR3(-0.577f, 0.577f, -0.577f);
	mDiffuseLight = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	mAmbientLight = D3DXCOLOR(0.6f, 0.6f, 0.6f, 1.0f);
	mSpecularLight = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

	mWhiteMtrl.ambient = _WHITE;
	mWhiteMtrl.diffuse = _WHITE;
	mWhiteMtrl.spec = _WHITE * 0.8f;
	mWhiteMtrl.specPower = 16.0f;

	mShadowMtrl.ambient = _BLACK;
	mShadowMtrl.diffuse = _BLACK;
	mShadowMtrl.diffuse.a = 0.5f; // 50% transparent.
	mShadowMtrl.spec = _BLACK;
	mShadowMtrl.specPower = 1.0f;


	D3DXMatrixIdentity(&mRoomWorld);
	D3DXMatrixTranslation(&mTeapotWorld, 0.0f, 3.0f, -6.0f);

	HR(D3DXCreateTextureFromFile(gD3dDevice, "checkboard.dds", &mFloorTex));
	HR(D3DXCreateTextureFromFile(gD3dDevice, "brick2.dds", &mWallTex));
	HR(D3DXCreateTextureFromFile(gD3dDevice, "ice.dds", &mMirrorTex));
	HR(D3DXCreateTextureFromFile(gD3dDevice, "brick1.dds", &mTeapotTex));

	HR(D3DXCreateTeapot(gD3dDevice, &mTeapot, 0));
	// Generate texture coordinates for the teapot.
	GenSphericalTexCoords();

	// Room geometry count.
	mGfxStats->AddVertices(24);
	mGfxStats->AddTriangles(8);

	// We draw the teapot 3 times--one regularly, one reflected and one shadowed.
	mGfxStats->AddVertices(mTeapot->GetNumVertices() * 3);
	mGfxStats->AddTriangles(mTeapot->GetNumFaces() * 3);

	BuildRoomGeometry();
	BuildFX();

	OnResetDevice();
}

ShadowDemoApp::~ShadowDemoApp()
{
	delete mGfxStats;
	ReleaseCOM(mRoomVB);
	ReleaseCOM(mTeapot);
	ReleaseCOM(mFloorTex);
	ReleaseCOM(mWallTex);
	ReleaseCOM(mMirrorTex);
	ReleaseCOM(mTeapotTex);
	ReleaseCOM(mFX);

	DestroyAllVertexDeclarations();
}

bool ShadowDemoApp::CheckDeviceCaps()
{
	D3DCAPS9 caps;
	HR(gD3dDevice->GetDeviceCaps(&caps));

	if (caps.VertexShaderVersion < D3DVS_VERSION(2, 0))
		return false;

	if (caps.PixelShaderVersion < D3DPS_VERSION(2, 0))
		return false;

	return true;
}

void ShadowDemoApp::OnLostDevice()
{
	mGfxStats->OnLostDevice();
	HR(mFX->OnLostDevice());
}

void ShadowDemoApp::OnResetDevice()
{
	mGfxStats->OnResetDevice();
	HR(mFX->OnResetDevice());

	BuildProjMtx();
}

void ShadowDemoApp::UpdateScene(float dt)
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

	if (gDInput->keyDown(DIK_A))
		mTeapotWorld(3, 0) -= 2.0f * dt;
	if (gDInput->keyDown(DIK_D))
		mTeapotWorld(3, 0) += 2.0f * dt;
}

void ShadowDemoApp::DrawScene()
{
	HR(gD3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 0xffeeeeee, 1.0f, 0));

	HR(gD3dDevice->BeginScene());

	HR(mFX->SetTechnique(mhTech));
	HR(mFX->SetValue(mhLightVecW, &mLightVecW, sizeof(D3DXVECTOR3)));
	HR(mFX->SetValue(mhDiffuseLight, &mDiffuseLight, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhAmbientLight, &mAmbientLight, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhSpecularLight, &mSpecularLight, sizeof(D3DXCOLOR)));

	HR(mFX->SetValue(mhAmbientMtrl, &mWhiteMtrl.ambient, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhDiffuseMtrl, &mWhiteMtrl.diffuse, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhSpecularMtrl, &mWhiteMtrl.spec, sizeof(D3DXCOLOR)));
	HR(mFX->SetFloat(mhSpecularPower, mWhiteMtrl.specPower));

	DrawRoom();
	DrawMirror();
	DrawTeapot();
	DrawReflectedTeapot();

	HR(mFX->SetValue(mhAmbientMtrl, &mShadowMtrl.ambient, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhDiffuseMtrl, &mShadowMtrl.diffuse, sizeof(D3DXCOLOR)));
	HR(mFX->SetValue(mhSpecularMtrl, &mShadowMtrl.spec, sizeof(D3DXCOLOR)));
	HR(mFX->SetFloat(mhSpecularPower, mShadowMtrl.specPower));
	DrawTeapotShadow();

	mGfxStats->Display();

	HR(gD3dDevice->EndScene());

	HR(gD3dDevice->Present(0, 0, 0, 0));
}

void ShadowDemoApp::BuildRoomGeometry()
{
	HR(gD3dDevice->CreateVertexBuffer(24 * sizeof(VertexPNT), D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &mRoomVB, 0));

	VertexPNT* v = NULL;
	HR(mRoomVB->Lock(0, 0, (void**)&v, 0));

	// Floor: Observe we tile texture coordinates.
	v[0] = VertexPNT(-7.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 4.0f);
	v[1] = VertexPNT(-7.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);
	v[2] = VertexPNT(7.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 4.0f, 0.0f);

	v[3] = VertexPNT(-7.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 4.0f);
	v[4] = VertexPNT(7.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 4.0f, 0.0f);
	v[5] = VertexPNT(7.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 4.0f, 4.0f);

	// Wall: Observe we tile texture coordinates, and that we
	// leave a gap in the middle for the mirror.
	v[6] = VertexPNT(-7.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f);
	v[7] = VertexPNT(-7.5f, 5.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[8] = VertexPNT(-2.5f, 5.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 0.0f);

	v[9] = VertexPNT(-7.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f);
	v[10] = VertexPNT(-2.5f, 5.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 0.0f);
	v[11] = VertexPNT(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 2.0f);

	v[12] = VertexPNT(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f);
	v[13] = VertexPNT(2.5f, 5.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[14] = VertexPNT(7.5f, 5.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 0.0f);

	v[15] = VertexPNT(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f);
	v[16] = VertexPNT(7.5f, 5.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 0.0f);
	v[17] = VertexPNT(7.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 2.0f);

	// Mirror
	v[18] = VertexPNT(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[19] = VertexPNT(-2.5f, 5.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[20] = VertexPNT(2.5f, 5.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);

	v[21] = VertexPNT(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[22] = VertexPNT(2.5f, 5.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	v[23] = VertexPNT(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

	HR(mRoomVB->Unlock());
}

void ShadowDemoApp::BuildFX()
{
	ID3DXBuffer* errors = 0;
	HR(D3DXCreateEffectFromFile(gD3dDevice, "DirLightTex.fx", 0, 0, D3DXSHADER_DEBUG, 0, &mFX, &errors));
	if (errors)
		MessageBox(0, (char*)errors->GetBufferPointer(), 0, 0);

	// Obtain handles.
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

void ShadowDemoApp::BuildViewMtx()
{
	float x = mCameraRadius * cosf(mCameraRotationY);
	float z = mCameraRadius * sinf(mCameraRotationY);
	D3DXVECTOR3 pos(x, mCameraHeight, z);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&mView, &pos, &target, &up);

	HR(mFX->SetValue(mhEyePos, &pos, sizeof(D3DXVECTOR3)));
}

void ShadowDemoApp::BuildProjMtx()
{
	float w = (float)mD3dpp.BackBufferWidth;
	float h = (float)mD3dpp.BackBufferHeight;
	D3DXMatrixPerspectiveFovLH(&mProj, D3DX_PI * 0.25f, w / h, 1.0f, 5000.0f);
}

void ShadowDemoApp::DrawRoom()
{
	HR(mFX->SetMatrix(mhWVP, &(mRoomWorld*mView*mProj)));
	D3DXMATRIX worldInvTrans;
	D3DXMatrixInverse(&worldInvTrans, 0, &mRoomWorld);
	D3DXMatrixTranspose(&worldInvTrans, &worldInvTrans);
	HR(mFX->SetMatrix(mhWorldInvTrans, &worldInvTrans));
	HR(mFX->SetMatrix(mhWorld, &mRoomWorld));

	HR(gD3dDevice->SetVertexDeclaration(VertexPNT::decl));
	HR(gD3dDevice->SetStreamSource(0, mRoomVB, 0, sizeof(VertexPNT)));

	// Begin passes.
	UINT numPasses = 0;
	HR(mFX->Begin(&numPasses, 0));
	for (UINT i = 0; i < numPasses; ++i)
	{
		HR(mFX->BeginPass(i));

		// draw the floor
		HR(mFX->SetTexture(mhTex, mFloorTex));
		HR(mFX->CommitChanges());
		HR(gD3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2));

		// draw the walls
		HR(mFX->SetTexture(mhTex, mWallTex));
		HR(mFX->CommitChanges());
		HR(gD3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 6, 4));

		HR(mFX->EndPass());
	}
	HR(mFX->End());
}

void ShadowDemoApp::DrawMirror()
{
	HR(mFX->SetMatrix(mhWVP, &(mRoomWorld*mView*mProj)));
	D3DXMATRIX worldInvTrans;
	D3DXMatrixInverse(&worldInvTrans, 0, &mRoomWorld);
	D3DXMatrixTranspose(&worldInvTrans, &worldInvTrans);
	HR(mFX->SetMatrix(mhWorldInvTrans, &worldInvTrans));
	HR(mFX->SetMatrix(mhWorld, &mRoomWorld));
	HR(mFX->SetTexture(mhTex, mMirrorTex));

	HR(gD3dDevice->SetVertexDeclaration(VertexPNT::decl));
	HR(gD3dDevice->SetStreamSource(0, mRoomVB, 0, sizeof(VertexPNT)));

	// Begin passes.
	UINT numPasses = 0;
	HR(mFX->Begin(&numPasses, 0));
	for (UINT i = 0; i < numPasses; ++i)
	{
		HR(mFX->BeginPass(i));

		// draw the mirror
		HR(gD3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 18, 2));

		HR(mFX->EndPass());
	}
	HR(mFX->End());
}

void ShadowDemoApp::DrawTeapot()
{
	HR(gD3dDevice->SetRenderState(D3DRS_WRAP0, D3DWRAPCOORD_0));

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

	// Disable wrap.
	HR(gD3dDevice->SetRenderState(D3DRS_WRAP0, 0));
}

void ShadowDemoApp::DrawReflectedTeapot()
{
	HR(gD3dDevice->SetRenderState(D3DRS_STENCILENABLE, true));
	HR(gD3dDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS));
	HR(gD3dDevice->SetRenderState(D3DRS_STENCILREF, 0x1));
	HR(gD3dDevice->SetRenderState(D3DRS_STENCILMASK, 0xffffffff));
	HR(gD3dDevice->SetRenderState(D3DRS_STENCILWRITEMASK, 0xffffffff));
	HR(gD3dDevice->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP));
	HR(gD3dDevice->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP));
	HR(gD3dDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE));

	// disable writes to the depth and back buffers
	HR(gD3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, false));
	HR(gD3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true));
	HR(gD3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO));
	HR(gD3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE));

	// Draw mirror to stencil only.
	DrawMirror();

	// Re-enable depth writes
	HR(gD3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, true));

	// Only draw reflected teapot to the pixels where the mirror
	// was drawn to.
	HR(gD3dDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_EQUAL));
	HR(gD3dDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP));

	// Build Reflection transformation.
	D3DXMATRIX R;
	D3DXPLANE plane(0.0f, 0.0f, 1.0f, 0.0f); // xy plane
	D3DXMatrixReflect(&R, &plane);

	// Save the original teapot world matrix.
	D3DXMATRIX oldTeapotWorld = mTeapotWorld;

	// Add reflection transform.
	mTeapotWorld = mTeapotWorld * R;

	// Reflect light vector also.
	D3DXVECTOR3 oldLightVecW = mLightVecW;
	D3DXVec3TransformNormal(&mLightVecW, &mLightVecW, &R);
	HR(mFX->SetValue(mhLightVecW, &mLightVecW, sizeof(D3DXVECTOR3)));

	// Disable depth buffer and render the reflected teapot.
	HR(gD3dDevice->SetRenderState(D3DRS_ZENABLE, false));
	HR(gD3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false));

	// Finally, draw the reflected teapot
	HR(gD3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW));
	DrawTeapot();
	mTeapotWorld = oldTeapotWorld;
	mLightVecW = oldLightVecW;

	// Restore render states.
	HR(gD3dDevice->SetRenderState(D3DRS_ZENABLE, true));
	HR(gD3dDevice->SetRenderState(D3DRS_STENCILENABLE, false));
	HR(gD3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW));
}

void ShadowDemoApp::DrawTeapotShadow()
{
	HR(gD3dDevice->SetRenderState(D3DRS_STENCILENABLE, true));
	HR(gD3dDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_EQUAL));
	HR(gD3dDevice->SetRenderState(D3DRS_STENCILREF, 0x0));
	HR(gD3dDevice->SetRenderState(D3DRS_STENCILMASK, 0xffffffff));
	HR(gD3dDevice->SetRenderState(D3DRS_STENCILWRITEMASK, 0xffffffff));
	HR(gD3dDevice->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP));
	HR(gD3dDevice->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP));
	HR(gD3dDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_INCR));

	D3DXVECTOR4 lightDirection(0.577f, -0.577f, 0.577f, 0.0f);
	D3DXPLANE groundPlane(0.0f, -1.0f, 0.0f, 0.0f);

	D3DXMATRIX S;
	D3DXMatrixShadow(&S, &lightDirection, &groundPlane);

	D3DXMATRIX eps;
	D3DXMatrixTranslation(&eps, 0.0f, 0.001f, 0.0f);

	D3DXMATRIX oldTeapotWorld = mTeapotWorld;
	mTeapotWorld = mTeapotWorld * S * eps;

	HR(gD3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true));
	HR(gD3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
	HR(gD3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));

	DrawTeapot();

	mTeapotWorld = oldTeapotWorld;
	HR(gD3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false));
	HR(gD3dDevice->SetRenderState(D3DRS_STENCILENABLE, false));
}

void ShadowDemoApp::GenSphericalTexCoords()
{
	D3DVERTEXELEMENT9 elements[64];
	UINT numElements = 0;
	VertexPNT::decl->GetDeclaration(elements, &numElements);

	ID3DXMesh* temp = 0;
	HR(mTeapot->CloneMesh(D3DXMESH_SYSTEMMEM, elements, gD3dDevice, &temp));

	ReleaseCOM(mTeapot);

	VertexPNT* vertices = NULL;
	HR(temp->LockVertexBuffer(0, (void**)&vertices));

	for (UINT i = 0; i < temp->GetNumVertices(); ++i)
	{
		// Convert to spherical coordinates.
		D3DXVECTOR3 p = vertices[i].pos;

		float theta = atan2f(p.z, p.x);
		float phi = acosf(p.y / sqrtf(p.x*p.x + p.y*p.y + p.z*p.z));

		float u = theta / (2.0f*D3DX_PI);
		float v = phi / D3DX_PI;

		// Save texture coordinates.

		vertices[i].tex0.x = u;
		vertices[i].tex0.y = v;
	}
	HR(temp->UnlockVertexBuffer());

	// Clone back to a hardware mesh.
	HR(temp->CloneMesh(D3DXMESH_MANAGED | D3DXMESH_WRITEONLY, elements, gD3dDevice, &mTeapot));

	ReleaseCOM(temp);
}
