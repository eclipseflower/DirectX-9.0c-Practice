#include "ShadowMapDemoApp.h"
#include "Vertex.h"
#include "Camera.h"
#include "DirectInput.h"

ShadowMapDemoApp::ShadowMapDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP) : D3DApp(hInstance, winCaption, devType, requestedVP)
{
	if (!CheckDeviceCaps())
	{
		MessageBox(0, "checkDeviceCaps() Failed", 0, 0);
		PostQuitMessage(0);
	}

	InitAllVertexDeclarations();

	mGfxStats = new GfxStats();
	mSky = new Sky("grassenvmap1024.dds", 10000.0f);

	LoadXFile("BasicColumnScene.x", &mSceneMesh, mSceneMtrls, mSceneTextures);
	D3DXMatrixIdentity(&mSceneWorld);

	LoadXFile("car.x", &mCarMesh, mCarMtrls, mCarTextures);
	D3DXMATRIX S, T;
	D3DXMatrixScaling(&S, 1.5f, 1.5f, 1.5f);
	D3DXMatrixTranslation(&T, 6.0f, 3.5f, -3.0f);
	mCarWorld = S * T;

	HR(D3DXCreateTextureFromFile(gD3dDevice, "whitetex.dds", &mWhiteTex));

	// Set some light properties; other properties are set in update function,
	// where they are animated.
	mLight.ambient = D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f);
	mLight.diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	mLight.spec = D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f);
	mLight.spotPower = 32.0f;

	// Create shadow map.
	D3DVIEWPORT9 vp = { 0, 0, 512, 512, 0.0f, 1.0f };
	mShadowMap = new DrawableTex2D(512, 512, 1, D3DFMT_R32F, true, D3DFMT_D24X8, vp, false);

	// Initialize camera.
	gCamera->Pos().y = 20.0f;
	gCamera->Pos().z = -100.0f;
	gCamera->SetSpeed(50.0f);

	mGfxStats->AddVertices(mSceneMesh->GetNumVertices());
	mGfxStats->AddTriangles(mSceneMesh->GetNumFaces());

	mGfxStats->AddVertices(mCarMesh->GetNumVertices());
	mGfxStats->AddTriangles(mCarMesh->GetNumFaces());

	mGfxStats->AddVertices(mSky->GetNumVertices());
	mGfxStats->AddTriangles(mSky->GetNumTriangles());

	BuildFX();

	OnResetDevice();
}

ShadowMapDemoApp::~ShadowMapDemoApp()
{
	delete mGfxStats;
	delete mSky;
	ReleaseCOM(mFX);
	ReleaseCOM(mWhiteTex);
	delete mShadowMap;

	ReleaseCOM(mSceneMesh);
	for (UINT i = 0; i < mSceneTextures.size(); ++i)
		ReleaseCOM(mSceneTextures[i]);

	ReleaseCOM(mCarMesh);
	for (UINT i = 0; i < mCarTextures.size(); ++i)
		ReleaseCOM(mCarTextures[i]);

	DestroyAllVertexDeclarations();
}

bool ShadowMapDemoApp::CheckDeviceCaps()
{
	D3DCAPS9 caps;
	HR(gD3dDevice->GetDeviceCaps(&caps));

	// Check for vertex shader version 2.0 support.
	if (caps.VertexShaderVersion < D3DVS_VERSION(2, 0))
		return false;

	// Check for pixel shader version 2.0 support.
	if (caps.PixelShaderVersion < D3DPS_VERSION(2, 0))
		return false;

	// Check render target support.  The adapter format can be either the display mode format
	// for windowed mode, or D3DFMT_X8R8G8B8 for fullscreen mode, so we need to test against
	// both.  We use D3DFMT_R32F as the render texture format and D3DFMT_D24X8 as the 
	// render texture depth format.
	D3DDISPLAYMODE mode;
	mD3dObject->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &mode);

	// Windowed.
	if (FAILED(mD3dObject->CheckDeviceFormat(D3DADAPTER_DEFAULT, mDevType, mode.Format,
		D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, D3DFMT_R32F)))
		return false;
	if (FAILED(mD3dObject->CheckDepthStencilMatch(D3DADAPTER_DEFAULT, mDevType, mode.Format,
		D3DFMT_R32F, D3DFMT_D24X8)))
		return false;

	// Fullscreen.
	if (FAILED(mD3dObject->CheckDeviceFormat(D3DADAPTER_DEFAULT, mDevType, D3DFMT_X8R8G8B8,
		D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, D3DFMT_R32F)))
		return false;
	if (FAILED(mD3dObject->CheckDepthStencilMatch(D3DADAPTER_DEFAULT, mDevType, D3DFMT_X8R8G8B8,
		D3DFMT_R32F, D3DFMT_D24X8)))
		return false;


	return true;
}

void ShadowMapDemoApp::OnLostDevice()
{
	mGfxStats->OnLostDevice();
	mSky->OnLostDevice();
	mShadowMap->OnLostDevice();
	HR(mFX->OnLostDevice());
}

void ShadowMapDemoApp::OnResetDevice()
{
	mGfxStats->OnResetDevice();
	mSky->OnResetDevice();
	mShadowMap->OnResetDevice();
	HR(mFX->OnResetDevice());

	// The aspect ratio depends on the backbuffer dimensions, which can 
	// possibly change after a reset.  So rebuild the projection matrix.
	float w = (float)mD3dpp.BackBufferWidth;
	float h = (float)mD3dpp.BackBufferHeight;
	gCamera->SetLens(D3DX_PI * 0.25f, w / h, 1.0f, 2000.0f);
}

void ShadowMapDemoApp::UpdateScene(float dt)
{
	mGfxStats->Update(dt);

	gDInput->poll();

	gCamera->Update(dt, 0, 0);

	// Animate spot light by rotating it on y-axis with respect to time.
	D3DXMATRIX lightView;
	D3DXVECTOR3 lightPosW(125.0f, 50.0f, 0.0f);
	D3DXVECTOR3 lightTargetW(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 lightUpW(0.0f, 1.0f, 0.0f);

	static float t = 0.0f;
	t += dt;
	if (t >= 2.0f*D3DX_PI)
		t = 0.0f;
	D3DXMATRIX Ry;
	D3DXMatrixRotationY(&Ry, t);
	D3DXVec3TransformCoord(&lightPosW, &lightPosW, &Ry);

	D3DXMatrixLookAtLH(&lightView, &lightPosW, &lightTargetW, &lightUpW);

	D3DXMATRIX lightLens;
	float lightFOV = D3DX_PI * 0.25f;
	D3DXMatrixPerspectiveFovLH(&lightLens, lightFOV, 1.0f, 1.0f, 200.0f);

	mLightVP = lightView * lightLens;

	// Setup a spotlight corresponding to the projector.
	D3DXVECTOR3 lightDirW = lightTargetW - lightPosW;
	D3DXVec3Normalize(&lightDirW, &lightDirW);
	mLight.posW = lightPosW;
	mLight.dirW = lightDirW;
}

void ShadowMapDemoApp::DrawScene()
{
	DrawShadowMap();

	HR(gD3dDevice->BeginScene());

	mSky->Draw();

	HR(mFX->SetTechnique(mhTech));

	HR(mFX->SetValue(mhLight, &mLight, sizeof(SpotLight)));
	HR(mFX->SetValue(mhEyePosW, &gCamera->Pos(), sizeof(D3DXVECTOR3)));
	HR(mFX->SetTexture(mhShadowMap, mShadowMap->D3dTex()));

	UINT numPasses = 0;
	HR(mFX->Begin(&numPasses, 0));
	HR(mFX->BeginPass(0));

	// Draw Scene mesh.
	HR(mFX->SetMatrix(mhLightWVP, &(mSceneWorld*mLightVP)));
	HR(mFX->SetMatrix(mhWVP, &(mSceneWorld*gCamera->ViewProj())));
	HR(mFX->SetMatrix(mhWorld, &mSceneWorld));
	for (UINT j = 0; j < mSceneMtrls.size(); ++j)
	{
		HR(mFX->SetValue(mhMtrl, &mSceneMtrls[j], sizeof(Mtrl)));

		if (mSceneTextures[j] != 0)
		{
			HR(mFX->SetTexture(mhTex, mSceneTextures[j]));
		}
		else
		{
			HR(mFX->SetTexture(mhTex, mWhiteTex));
		}

		HR(mFX->CommitChanges());
		HR(mSceneMesh->DrawSubset(j));
	}

	// Draw car mesh.
	HR(mFX->SetMatrix(mhLightWVP, &(mCarWorld*mLightVP)));
	HR(mFX->SetMatrix(mhWVP, &(mCarWorld*gCamera->ViewProj())));
	HR(mFX->SetMatrix(mhWorld, &mCarWorld));
	for (UINT j = 0; j < mCarMtrls.size(); ++j)
	{
		HR(mFX->SetValue(mhMtrl, &mCarMtrls[j], sizeof(Mtrl)));

		if (mCarTextures[j] != 0)
		{
			HR(mFX->SetTexture(mhTex, mCarTextures[j]));
		}
		else
		{
			HR(mFX->SetTexture(mhTex, mWhiteTex));
		}

		HR(mFX->CommitChanges());
		HR(mCarMesh->DrawSubset(j));
	}

	HR(mFX->EndPass());
	HR(mFX->End());

	mGfxStats->Display();

	HR(gD3dDevice->EndScene());

	// Present the backbuffer.
	HR(gD3dDevice->Present(0, 0, 0, 0));
}

void ShadowMapDemoApp::DrawShadowMap()
{
	mShadowMap->BeginScene();
	HR(gD3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0));

	HR(mFX->SetTechnique(mhBuildShadowMapTech));

	UINT numPasses = 0;
	HR(mFX->Begin(&numPasses, 0));
	HR(mFX->BeginPass(0));

	// Draw scene mesh.
	HR(mFX->SetMatrix(mhLightWVP, &(mSceneWorld*mLightVP)));
	HR(mFX->CommitChanges());
	for (UINT j = 0; j < mSceneMtrls.size(); ++j)
	{
		for (UINT j = 0; j < mSceneMtrls.size(); ++j)
		{
			HR(mSceneMesh->DrawSubset(j));
		}
	}

	// Draw car mesh.
	HR(mFX->SetMatrix(mhLightWVP, &(mCarWorld*mLightVP)));
	HR(mFX->CommitChanges());
	for (UINT j = 0; j < mCarMtrls.size(); ++j)
	{
		for (UINT j = 0; j < mCarMtrls.size(); ++j)
		{
			HR(mCarMesh->DrawSubset(j));
		}
	}


	HR(mFX->EndPass());
	HR(mFX->End());

	mShadowMap->EndScene();
}

void ShadowMapDemoApp::BuildFX()
{
	ID3DXBuffer* errors = 0;
	HR(D3DXCreateEffectFromFile(gD3dDevice, "lightshadow.fx",
		0, 0, D3DXSHADER_DEBUG, 0, &mFX, &errors));
	if (errors)
		MessageBox(0, (char*)errors->GetBufferPointer(), 0, 0);

	// Obtain handles.
	mhTech = mFX->GetTechniqueByName("LightShadowTech");
	mhBuildShadowMapTech = mFX->GetTechniqueByName("BuildShadowMapTech");
	mhLightWVP = mFX->GetParameterByName(0, "gLightWVP");
	mhWVP = mFX->GetParameterByName(0, "gWVP");
	mhWorld = mFX->GetParameterByName(0, "gWorld");
	mhMtrl = mFX->GetParameterByName(0, "gMtrl");
	mhLight = mFX->GetParameterByName(0, "gLight");
	mhEyePosW = mFX->GetParameterByName(0, "gEyePosW");
	mhTex = mFX->GetParameterByName(0, "gTex");
	mhShadowMap = mFX->GetParameterByName(0, "gShadowMap");
}
