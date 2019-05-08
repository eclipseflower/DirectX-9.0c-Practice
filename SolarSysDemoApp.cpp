#include "DirectInput.h"
#include "Vertex.h"
#include "SolarSysDemoApp.h"

SolarSysDemoApp::SolarSysDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP) : D3DApp(hInstance, winCaption, devType, requestedVP)
{
	if (!CheckDeviceCaps())
	{
		MessageBox(0, "checkDeviceCaps() Failed", 0, 0);
		PostQuitMessage(0);
	}

	InitAllVertexDeclarations();

	mGfxStats = new GfxStats();

	// Initialize Camera Settings
	mCameraRadius = 25.0f;
	mCameraRotationY = 1.2f * D3DX_PI;
	mCameraHeight = 10.0f;

	// Setup a directional light.
	mLight.dirW = D3DXVECTOR3(0.0f, 1.0f, 2.0f);
	D3DXVec3Normalize(&mLight.dirW, &mLight.dirW);
	mLight.ambient = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	mLight.diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	mLight.spec = D3DXCOLOR(0.6f, 0.6f, 0.6f, 1.0f);

	// Create a sphere to represent a solar object.
	HR(D3DXCreateSphere(gD3dDevice, 1.0f, 30, 30, &mSphere, 0));
	GenSphericalTexCoords();
	D3DXMatrixIdentity(&mWorld);

	// Create the textures.
	HR(D3DXCreateTextureFromFile(gD3dDevice, "sun.dds", &mSunTex));
	HR(D3DXCreateTextureFromFile(gD3dDevice, "planet1.dds", &mPlanet1Tex));
	HR(D3DXCreateTextureFromFile(gD3dDevice, "planet2.dds", &mPlanet2Tex));
	HR(D3DXCreateTextureFromFile(gD3dDevice, "planet3.dds", &mPlanet3Tex));
	HR(D3DXCreateTextureFromFile(gD3dDevice, "moon.dds", &mMoonTex));

	// Initialize default white material.
	mWhiteMtrl.ambient = _WHITE;
	mWhiteMtrl.diffuse = _WHITE;
	mWhiteMtrl.spec = _WHITE * 0.5f;
	mWhiteMtrl.specPower = 48.0f;

	//==================================================
	// Specify how the solar object frames are related.

	D3DXVECTOR3 pos[NUM_OBJECTS] =
	{
		D3DXVECTOR3(0.0f, 0.0f, 0.0f),
		D3DXVECTOR3(7.0f, 0.0f, 7.0f),
		D3DXVECTOR3(-9.0f, 0.0f, 0.0f),
		D3DXVECTOR3(7.0f, 0.0f, -6.0f),
		D3DXVECTOR3(5.0f, 0.0f, 0.0f),
		D3DXVECTOR3(-5.0f, 0.0f, 0.0f),
		D3DXVECTOR3(3.0f, 0.0f, 0.0f),
		D3DXVECTOR3(2.0f, 0.0f, -2.0f),
		D3DXVECTOR3(-2.0f, 0.0f, 0.0f),
		D3DXVECTOR3(0.0f, 0.0f, 2.0f)
	};

	mObject[0].Set(SUN, pos[0], 0.0f, -1, 2.5f, mSunTex);  // Sun
	mObject[1].Set(PLANET, pos[1], 0.0f, 0, 1.5f, mPlanet1Tex);// P1
	mObject[2].Set(PLANET, pos[2], 0.0f, 0, 1.2f, mPlanet2Tex);// P2
	mObject[3].Set(PLANET, pos[3], 0.0f, 0, 0.8f, mPlanet3Tex);// P3

	mObject[4].Set(MOON, pos[4], 0.0f, 1, 0.5f, mMoonTex); // M1P1
	mObject[5].Set(MOON, pos[5], 0.0f, 1, 0.5f, mMoonTex); // M2P1
	mObject[6].Set(MOON, pos[6], 0.0f, 2, 0.4f, mMoonTex); // M1P2
	mObject[7].Set(MOON, pos[7], 0.0f, 3, 0.3f, mMoonTex); // M1P3
	mObject[8].Set(MOON, pos[8], 0.0f, 3, 0.3f, mMoonTex); // M2P3
	mObject[9].Set(MOON, pos[9], 0.0f, 3, 0.3f, mMoonTex); // M3P3


	//==================================================

	mGfxStats->AddVertices(mSphere->GetNumVertices() * NUM_OBJECTS);
	mGfxStats->AddTriangles(mSphere->GetNumFaces() * NUM_OBJECTS);

	BuildFX();

	OnResetDevice();
}

SolarSysDemoApp::~SolarSysDemoApp()
{
	delete mGfxStats;

	ReleaseCOM(mFX);
	ReleaseCOM(mSphere);
	ReleaseCOM(mSunTex);
	ReleaseCOM(mPlanet1Tex);
	ReleaseCOM(mPlanet2Tex);
	ReleaseCOM(mPlanet3Tex);
	ReleaseCOM(mMoonTex);

	DestroyAllVertexDeclarations();
}

bool SolarSysDemoApp::CheckDeviceCaps()
{
	D3DCAPS9 caps;
	HR(gD3dDevice->GetDeviceCaps(&caps));

	// Check for vertex shader version 2.0 support.
	if (caps.VertexShaderVersion < D3DVS_VERSION(2, 0))
		return false;

	// Check for pixel shader version 2.0 support.
	if (caps.PixelShaderVersion < D3DPS_VERSION(2, 0))
		return false;

	return true;
}

void SolarSysDemoApp::OnLostDevice()
{
	mGfxStats->OnLostDevice();
	HR(mFX->OnLostDevice());
}

void SolarSysDemoApp::OnResetDevice()
{
	mGfxStats->OnResetDevice();
	HR(mFX->OnResetDevice());

	// The aspect ratio depends on the backbuffer dimensions, which can 
	// possibly change after a reset.  So rebuild the projection matrix.
	BuildProjMtx();
}

void SolarSysDemoApp::UpdateScene(float dt)
{
	mGfxStats->Update(dt);

	// Get snapshot of input devices.
	gDInput->poll();

	// Check input.
	if (gDInput->keyDown(DIK_W))
		mCameraHeight += 25.0f * dt;
	if (gDInput->keyDown(DIK_S))
		mCameraHeight -= 25.0f * dt;

	// Divide by 50 to make mouse less sensitive. 
	mCameraRotationY += gDInput->mouseDX() / 100.0f;
	mCameraRadius += gDInput->mouseDY() / 25.0f;

	// If we rotate over 360 degrees, just roll back to 0
	if (fabsf(mCameraRotationY) >= 2.0f * D3DX_PI)
		mCameraRotationY = 0.0f;

	// Don't let radius get too small.
	if (mCameraRadius < 2.0f)
		mCameraRadius = 2.0f;

	// The camera position/orientation relative to world space can 
	// change every frame based on input, so we need to rebuild the
	// view matrix every frame with the latest changes.
	BuildViewMtx();

	//================================================
	// Animate the solar objects with respect to time.

	for (int i = 0; i < NUM_OBJECTS; ++i)
	{
		switch (mObject[i].typeID)
		{
		case SUN:
			mObject[i].yAngle += 1.5f * dt;
			break;
		case PLANET:
			mObject[i].yAngle += 2.0f * dt;
			break;
		case MOON:
			mObject[i].yAngle += 2.5f * dt;
			break;
		}

		// If we rotate over 360 degrees, just roll back to 0.
		if (mObject[i].yAngle >= 2.0f*D3DX_PI)
			mObject[i].yAngle = 0.0f;
	}
}

void SolarSysDemoApp::DrawScene()
{
	HR(gD3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xff000000, 1.0f, 0));

	HR(gD3dDevice->BeginScene());

	HR(mFX->SetValue(mhLight, &mLight, sizeof(DirLight)));

	HR(mFX->SetTechnique(mhTech));
	UINT numPasses = 0;
	HR(mFX->Begin(&numPasses, 0));
	HR(mFX->BeginPass(0));

	// Wrap the texture coordinates that get assigned to TEXCOORD2 in the pixel shader.
	HR(gD3dDevice->SetRenderState(D3DRS_WRAP2, D3DWRAP_U));

	// Build the world transforms for each frame, then render them.
	BuildObjectWorldTransforms();
	D3DXMATRIX S;
	for (int i = 0; i < NUM_OBJECTS; ++i)
	{
		float s = mObject[i].size;
		D3DXMatrixScaling(&S, s, s, s);

		// Prefix the frame matrix with a scaling transformation to
		// size it relative to the world.
		mWorld = S * mObject[i].toWorldXForm;
		HR(mFX->SetMatrix(mhWVP, &(mWorld*mView*mProj)));
		D3DXMATRIX worldInvTrans;
		D3DXMatrixInverse(&worldInvTrans, 0, &mWorld);
		D3DXMatrixTranspose(&worldInvTrans, &worldInvTrans);
		HR(mFX->SetMatrix(mhWorldInvTrans, &worldInvTrans));
		HR(mFX->SetMatrix(mhWorld, &mWorld));
		HR(mFX->SetValue(mhMtrl, &mWhiteMtrl, sizeof(Mtrl)));
		HR(mFX->SetTexture(mhTex, mObject[i].tex));
		HR(mFX->CommitChanges());

		mSphere->DrawSubset(0);
	}
	HR(gD3dDevice->SetRenderState(D3DRS_WRAP2, 0));
	HR(mFX->EndPass());
	HR(mFX->End());

	mGfxStats->Display();

	HR(gD3dDevice->EndScene());

	// Present the backbuffer.
	HR(gD3dDevice->Present(0, 0, 0, 0));
}

void SolarSysDemoApp::BuildFX()
{
	ID3DXBuffer* errors = 0;
	HR(D3DXCreateEffectFromFile(gD3dDevice, "phongdirlttex.fx", 0, 0, D3DXSHADER_DEBUG, 0, &mFX, &errors));
	if (errors)
		MessageBox(0, (char*)errors->GetBufferPointer(), 0, 0);

	// Obtain handles.
	mhTech = mFX->GetTechniqueByName("PhongDirLtTexTech");
	mhWVP = mFX->GetParameterByName(0, "gWVP");
	mhWorldInvTrans = mFX->GetParameterByName(0, "gWorldInvTrans");
	mhMtrl = mFX->GetParameterByName(0, "gMtrl");
	mhLight = mFX->GetParameterByName(0, "gLight");
	mhEyePos = mFX->GetParameterByName(0, "gEyePosW");
	mhWorld = mFX->GetParameterByName(0, "gWorld");
	mhTex = mFX->GetParameterByName(0, "gTex");
}

void SolarSysDemoApp::BuildViewMtx()
{
	float x = mCameraRadius * cosf(mCameraRotationY);
	float z = mCameraRadius * sinf(mCameraRotationY);
	D3DXVECTOR3 pos(x, mCameraHeight, z);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&mView, &pos, &target, &up);

	HR(mFX->SetValue(mhEyePos, &pos, sizeof(D3DXVECTOR3)));
}

void SolarSysDemoApp::BuildProjMtx()
{
	float w = (float)mD3dpp.BackBufferWidth;
	float h = (float)mD3dpp.BackBufferHeight;
	D3DXMatrixPerspectiveFovLH(&mProj, D3DX_PI * 0.25f, w / h, 1.0f, 5000.0f);
}

void SolarSysDemoApp::BuildObjectWorldTransforms()
{
	D3DXMATRIX R, T;
	D3DXVECTOR3 p;
	for (int i = 0; i < NUM_OBJECTS; ++i)
	{
		p = mObject[i].pos;
		D3DXMatrixRotationY(&R, mObject[i].yAngle);
		D3DXMatrixTranslation(&T, p.x, p.y, p.z);
		mObject[i].toParentXForm = R * T;
	}

	// For each object...
	for (int i = 0; i < NUM_OBJECTS; ++i)
	{
		// Initialize to identity matrix.
		D3DXMatrixIdentity(&mObject[i].toWorldXForm);

		// The ith object's world transform is given by its 
		// to-parent transform, followed by its parent's to-parent transform, 
		// followed by its grandparent's to-parent transform, and
		// so on, up to the root's to-parent transform.
		int k = i;
		while (k != -1)
		{
			mObject[i].toWorldXForm *= mObject[k].toParentXForm;
			k = mObject[k].parent;
		}
	}
}

void SolarSysDemoApp::GenSphericalTexCoords()
{
	D3DVERTEXELEMENT9 elements[64];
	UINT numElements = 0;
	VertexPNT::decl->GetDeclaration(elements, &numElements);

	ID3DXMesh* temp = 0;
	HR(mSphere->CloneMesh(D3DXMESH_SYSTEMMEM, elements, gD3dDevice, &temp));

	ReleaseCOM(mSphere);

	// Now generate texture coordinates for each vertex.
	VertexPNT* vertices = 0;
	HR(temp->LockVertexBuffer(0, (void**)&vertices));

	for (UINT i = 0; i < temp->GetNumVertices(); ++i)
	{
		// Convert to spherical coordinates.
		D3DXVECTOR3 p = vertices[i].pos;


		float theta = atan2f(p.z, p.x);
		float phi = acosf(p.y / sqrtf(p.x*p.x + p.y*p.y + p.z*p.z));

		// Phi and theta give the texture coordinates, but are not in 
		// the range [0, 1], so scale them into that range.

		float u = theta / (2.0f*D3DX_PI);
		float v = phi / D3DX_PI;

		// Save texture coordinates.

		vertices[i].tex0.x = u;
		vertices[i].tex0.y = v;
	}
	HR(temp->UnlockVertexBuffer());

	// Clone back to a hardware mesh.
	HR(temp->CloneMesh(D3DXMESH_MANAGED | D3DXMESH_WRITEONLY, elements, gD3dDevice, &mSphere));

	ReleaseCOM(temp);
}
