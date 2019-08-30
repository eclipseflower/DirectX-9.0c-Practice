#include "AsteroidsDemoApp.h"
#include "Camera.h"
#include "DirectInput.h"

AsteroidsDemoApp::AsteroidsDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP) : D3DApp(hInstance, winCaption, devType, requestedVP)
{
	if (!CheckDeviceCaps())
	{
		MessageBox(0, "checkDeviceCaps() Failed", 0, 0);
		PostQuitMessage(0);
	}

	InitAllVertexDeclarations();

	mGfxStats = new GfxStats();

	// Load the asteroid mesh and compute its bounding box in local space.
	LoadXFile("asteroid.x", &mAsteroidMesh, mAsteroidMtrls, mAsteroidTextures);
	VertexPNT* v = 0;
	HR(mAsteroidMesh->LockVertexBuffer(0, (void**)&v));
	HR(D3DXComputeBoundingBox(&v->pos, mAsteroidMesh->GetNumVertices(),
		mAsteroidMesh->GetNumBytesPerVertex(),
		&mAsteroidBox.minPt, &mAsteroidBox.maxPt));
	HR(mAsteroidMesh->UnlockVertexBuffer());

	// Initialize camera.
	gCamera->Pos() = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	gCamera->SetSpeed(40.0f);

	// Initialize the particle system.
	D3DXMATRIX psysWorld;
	D3DXMatrixIdentity(&psysWorld);

	AABB psysBox;
	psysBox.maxPt = D3DXVECTOR3(INFINITY, INFINITY, INFINITY);
	psysBox.minPt = D3DXVECTOR3(-INFINITY, -INFINITY, -INFINITY);
	mFireWork = new FireWork("fireworks.fx", "FireWorksTech", "bolt.dds",
		D3DXVECTOR3(0.0f, -9.8f, 0.0f), psysBox, 500, -1.0f);
	mFireWork->SetWorldMtx(psysWorld);

	// Call update once to put all particles in the "alive" list.
	mFireWork->Update(0.0f);

	// Load the default texture.
	HR(D3DXCreateTextureFromFile(gD3dDevice, "whitetex.dds", &mWhiteTex));

	// Init a light.
	mLight.dirW = D3DXVECTOR3(0.707f, 0.0f, 0.707f);
	D3DXVec3Normalize(&mLight.dirW, &mLight.dirW);
	mLight.ambient = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	mLight.diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	mLight.spec = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

	BuildFX();
	InitAsteroids();

	OnResetDevice();
}

AsteroidsDemoApp::~AsteroidsDemoApp()
{
	delete mGfxStats;
	ReleaseCOM(mWhiteTex);
	ReleaseCOM(mFX);
	delete mFireWork;

	ReleaseCOM(mAsteroidMesh);
	for (UINT i = 0; i < mAsteroidTextures.size(); ++i)
		ReleaseCOM(mAsteroidTextures[i]);

	DestroyAllVertexDeclarations();
}

bool AsteroidsDemoApp::CheckDeviceCaps()
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

void AsteroidsDemoApp::OnLostDevice()
{
	mGfxStats->OnLostDevice();
	HR(mFX->OnLostDevice());
	mFireWork->OnLostDevice();
}

void AsteroidsDemoApp::OnResetDevice()
{
	mGfxStats->OnResetDevice();
	HR(mFX->OnResetDevice());
	mFireWork->OnResetDevice();

	// The aspect ratio depends on the backbuffer dimensions, which can 
	// possibly change after a reset.  So rebuild the projection matrix.
	float w = (float)mD3dpp.BackBufferWidth;
	float h = (float)mD3dpp.BackBufferHeight;
	gCamera->SetLens(D3DX_PI * 0.25f, w / h, 0.01f, 5000.0f);
}

void AsteroidsDemoApp::UpdateScene(float dt)
{
	mGfxStats->Update(dt);

	mGfxStats->SetTriCount(mAsteroidMesh->GetNumFaces()*mAsteroids.size());
	mGfxStats->SetVertexCount(mAsteroidMesh->GetNumVertices()*mAsteroids.size());

	gDInput->poll();

	gCamera->Update(dt, 0, 0);

	// Update the asteroids' orientation and position.
	std::list<Asteroid>::iterator asteroidIter = mAsteroids.begin();
	while (asteroidIter != mAsteroids.end())
	{
		asteroidIter->theta += 4.0f*dt;
		asteroidIter->pos += asteroidIter->vel*dt;
		++asteroidIter;
	}


	// Update and delete dead firework systems.
	std::list<FireWorkInstance>::iterator fireworkIter = mFireWorkInstances.begin();
	while (fireworkIter != mFireWorkInstances.end())
	{
		fireworkIter->time += dt;

		// Kill system after 1 seconds.
		if (fireworkIter->time > 1.0f)
			fireworkIter = mFireWorkInstances.erase(fireworkIter);
		else
			++fireworkIter;
	}
}

void AsteroidsDemoApp::DrawScene()
{
	HR(gD3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xff333333, 1.0f, 0));

	HR(gD3dDevice->BeginScene());

	HR(mFX->SetValue(mhEyePos, &gCamera->Pos(), sizeof(D3DXVECTOR3)));
	HR(mFX->SetTechnique(mhTech));
	UINT numPasses = 0;
	HR(mFX->Begin(&numPasses, 0));
	HR(mFX->BeginPass(0));

	// Did we pick anything?
	D3DXVECTOR3 originW(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 dirW(0.0f, 0.0f, 0.0f);
	if (gDInput->mouseButtonDown(0))
	{
		GetWorldPickingRay(originW, dirW);
	}

	std::list<Asteroid>::iterator iter = mAsteroids.begin();
	while (iter != mAsteroids.end())
	{
		// Build world matrix based on current rotation and position settings.
		D3DXMATRIX R, T;
		D3DXMatrixRotationAxis(&R, &iter->axis, iter->theta);
		D3DXMatrixTranslation(&T, iter->pos.x, iter->pos.y, iter->pos.z);

		D3DXMATRIX toWorld = R * T;

		// Transform AABB to world space.
		AABB box;
		mAsteroidBox.Xform(toWorld, box);

		// Did we pick it?
		if (D3DXBoxBoundProbe(&box.minPt, &box.maxPt, &originW, &dirW))
		{
			// Create a firework instance.
			FireWorkInstance inst;
			inst.time = 0.0f;
			inst.toWorld = toWorld;
			mFireWorkInstances.push_back(inst);

			// Remove asteroid from list and move onto the next node.
			iter = mAsteroids.erase(iter);
			continue;
		}

		// Only draw if AABB is visible.
		if (gCamera->IsVisible(box))
		{
			HR(mFX->SetMatrix(mhWVP, &(toWorld*gCamera->ViewProj())));
			D3DXMATRIX worldInvTrans;
			D3DXMatrixInverse(&worldInvTrans, 0, &toWorld);
			D3DXMatrixTranspose(&worldInvTrans, &worldInvTrans);
			HR(mFX->SetMatrix(mhWorldInvTrans, &worldInvTrans));
			HR(mFX->SetMatrix(mhWorld, &toWorld));

			for (UINT j = 0; j < mAsteroidMtrls.size(); ++j)
			{
				HR(mFX->SetValue(mhMtrl, &mAsteroidMtrls[j], sizeof(Mtrl)));

				// If there is a texture, then use.
				if (mAsteroidTextures[j] != 0)
				{
					HR(mFX->SetTexture(mhTex, mAsteroidTextures[j]));
				}

				// But if not, then set a pure white texture.  When the texture color
				// is multiplied by the color from lighting, it is like multiplying by
				// 1 and won't change the color from lighting.
				else
				{
					HR(mFX->SetTexture(mhTex, mWhiteTex));
				}

				HR(mFX->CommitChanges());
				HR(mAsteroidMesh->DrawSubset(j));
			}
		}
		++iter;
	}

	HR(mFX->EndPass());
	HR(mFX->End());

	// Draw fireworks.
	std::list<FireWorkInstance>::iterator psysIter = mFireWorkInstances.begin();
	while (psysIter != mFireWorkInstances.end())
	{
		mFireWork->SetTime(psysIter->time);
		mFireWork->SetWorldMtx(psysIter->toWorld);
		mFireWork->Draw();
		++psysIter;
	}

	mGfxStats->Display();

	HR(gD3dDevice->EndScene());

	// Present the backbuffer.
	HR(gD3dDevice->Present(0, 0, 0, 0));
}

void AsteroidsDemoApp::InitAsteroids()
{
	Asteroid a;
	for (int i = 0; i < NUM_ASTEROIDS; ++i)
	{
		// Generate a random rotation axis.
		GetRandomVec(a.axis);

		// No rotation to start, but we will rotate as 
		// a function of time.
		a.theta = 0.0f;

		// Random position in world space.
		a.pos.x = GetRandomFloat(-500.0f, 500.0f);
		a.pos.y = GetRandomFloat(-500.0f, 500.0f);
		a.pos.z = GetRandomFloat(-500.0f, 500.0f);

		// Random velocity in world space.
		float speed = GetRandomFloat(10.0f, 20.0f);
		D3DXVECTOR3 dir;
		GetRandomVec(dir);
		a.vel = speed * dir;

		mAsteroids.push_back(a);
	}
}

void AsteroidsDemoApp::BuildFX()
{
	ID3DXBuffer* errors = 0;
	HR(D3DXCreateEffectFromFile(gD3dDevice, "phongdirlttex.fx",
		0, 0, D3DXSHADER_DEBUG, 0, &mFX, &errors));
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

	HR(mFX->SetValue(mhLight, &mLight, sizeof(DirLight)));
}

void AsteroidsDemoApp::GetWorldPickingRay(D3DXVECTOR3 & originW, D3DXVECTOR3 & dirW)
{// Get the screen point clicked.
	POINT s;
	GetCursorPos(&s);

	// Make it relative to the client area window.
	ScreenToClient(mMainWnd, &s);

	// By the way we've been constructing things, the entire 
	// backbuffer is the viewport.

	float w = (float)mD3dpp.BackBufferWidth;
	float h = (float)mD3dpp.BackBufferHeight;

	D3DXMATRIX proj = gCamera->Proj();

	float x = (2.0f*s.x / w - 1.0f) / proj(0, 0);
	float y = (-2.0f*s.y / h + 1.0f) / proj(1, 1);

	// Build picking ray in view space.
	D3DXVECTOR3 origin(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 dir(x, y, 1.0f);

	// So if the view matrix transforms coordinates from 
	// world space to view space, then the inverse of the
	// view matrix transforms coordinates from view space
	// to world space.
	D3DXMATRIX invView;
	D3DXMatrixInverse(&invView, 0, &gCamera->View());

	// Transform picking ray to world space.
	D3DXVec3TransformCoord(&originW, &origin, &invView);
	D3DXVec3TransformNormal(&dirW, &dir, &invView);
	D3DXVec3Normalize(&dirW, &dirW);
}
