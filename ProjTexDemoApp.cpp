#include "ProjTexDemoApp.h"
#include "Vertex.h"
#include "Camera.h"
#include "DirectInput.h"

ProjTexDemoApp::ProjTexDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP) : D3DApp(hInstance, winCaption, devType, requestedVP)
{
	if (!CheckDeviceCaps())
	{
		MessageBox(0, "checkDeviceCaps() Failed", 0, 0);
		PostQuitMessage(0);
	}

	InitAllVertexDeclarations();

	mGfxStats = new GfxStats();
	mSky = new Sky("grassenvmap1024.dds", 10000.0f);

	LoadXFile("shapes.x", &mSceneMesh, mSceneMtrls, mSceneTextures);
	D3DXMatrixIdentity(&mSceneWorld);

	HR(D3DXCreateTextureFromFile(gD3dDevice, "whitetex.dds", &mWhiteTex));
	HR(D3DXCreateTextureFromFile(gD3dDevice, "skull.dds", &mSkullTex));

	// Build light projective texture matrix.
	D3DXMATRIX lightView;
	D3DXVECTOR3 lightPosW(60.0f, 90.0f, 0.0f);
	D3DXVECTOR3 lightTargetW(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 lightUpW(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&lightView, &lightPosW, &lightTargetW, &lightUpW);

	D3DXMATRIX lightLens;
	float lightFOV = D3DX_PI * 0.30f;
	D3DXMatrixPerspectiveFovLH(&lightLens, lightFOV, 1.0f, 1.0f, 200.0f);

	mLightWVP = mSceneWorld * lightView*lightLens;

	// Setup a spotlight corresponding to the projector.
	D3DXVECTOR3 lightDirW = lightTargetW - lightPosW;
	D3DXVec3Normalize(&lightDirW, &lightDirW);
	mLight.posW = lightPosW;
	mLight.dirW = lightDirW;
	mLight.ambient = D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f);
	mLight.diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	mLight.spec = D3DXCOLOR(0.8f, 0.8f, 0.8f, 1.0f);
	mLight.spotPower = 8.0f;

	// Initialize camera.
	gCamera->Pos().y = 100.0f;
	gCamera->Pos().z = -100.0f;
	gCamera->SetSpeed(50.0f);

	mGfxStats->AddVertices(mSceneMesh->GetNumVertices());
	mGfxStats->AddTriangles(mSceneMesh->GetNumFaces());
			   
	mGfxStats->AddVertices(mSky->GetNumVertices());
	mGfxStats->AddTriangles(mSky->GetNumTriangles());

	BuildFX();

	OnResetDevice();
}

ProjTexDemoApp::~ProjTexDemoApp()
{
	delete mGfxStats;
	delete mSky;
	ReleaseCOM(mFX);
	ReleaseCOM(mWhiteTex);
	ReleaseCOM(mSkullTex);

	ReleaseCOM(mSceneMesh);
	for (UINT i = 0; i < mSceneTextures.size(); ++i)
		ReleaseCOM(mSceneTextures[i]);

	DestroyAllVertexDeclarations();
}

bool ProjTexDemoApp::CheckDeviceCaps()
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

void ProjTexDemoApp::OnLostDevice()
{
	mGfxStats->OnLostDevice();
	mSky->OnLostDevice();
	HR(mFX->OnLostDevice());
}

void ProjTexDemoApp::OnResetDevice()
{
	mGfxStats->OnResetDevice();
	mSky->OnResetDevice();
	HR(mFX->OnResetDevice());

	// The aspect ratio depends on the backbuffer dimensions, which can 
	// possibly change after a reset.  So rebuild the projection matrix.
	float w = (float)mD3dpp.BackBufferWidth;
	float h = (float)mD3dpp.BackBufferHeight;
	gCamera->SetLens(D3DX_PI * 0.25f, w / h, 1.0f, 2000.0f);
}

void ProjTexDemoApp::UpdateScene(float dt)
{
	mGfxStats->Update(dt);

	gDInput->poll();

	gCamera->Update(dt, 0, 0);
}

void ProjTexDemoApp::DrawScene()
{
	HR(gD3dDevice->BeginScene());

	// Draw sky first--this also replaces our gd3dDevice->Clear call.
	//mSky->draw();
	HR(gD3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0));

	// Draw the scene mesh.
	HR(mFX->SetTechnique(mhTech));
	HR(mFX->SetMatrix(mhWorldInvTrans, &mSceneWorld));
	HR(mFX->SetMatrix(mhWorld, &mSceneWorld));
	HR(mFX->SetValue(mhLight, &mLight, sizeof(SpotLight)));
	HR(mFX->SetMatrix(mhWVP, &(mSceneWorld*gCamera->ViewProj())));
	HR(mFX->SetValue(mhEyePosW, &gCamera->Pos(), sizeof(D3DXVECTOR3)));
	HR(mFX->SetTexture(mhTex, mSkullTex));
	HR(mFX->SetMatrix(mhLightWVP, &mLightWVP));

	UINT numPasses = 0;
	HR(mFX->Begin(&numPasses, 0));
	HR(mFX->BeginPass(0));

	for (UINT j = 0; j < mSceneMtrls.size(); ++j)
	{
		HR(mFX->SetValue(mhMtrl, &mSceneMtrls[j], sizeof(Mtrl)));

		HR(mFX->CommitChanges());
		HR(mSceneMesh->DrawSubset(j));
	}

	HR(mFX->EndPass());
	HR(mFX->End());

	mGfxStats->Display();

	HR(gD3dDevice->EndScene());

	// Present the backbuffer.
	HR(gD3dDevice->Present(0, 0, 0, 0));
}

void ProjTexDemoApp::BuildFX()
{
	ID3DXBuffer* errors = 0;
	HR(D3DXCreateEffectFromFile(gD3dDevice, "projtex.fx",
		0, 0, D3DXSHADER_DEBUG | D3DXSHADER_SKIPOPTIMIZATION, 0, &mFX, &errors));
	if (errors)
		MessageBox(0, (char*)errors->GetBufferPointer(), 0, 0);

	// Obtain handles.
	mhTech = mFX->GetTechniqueByName("ProjTexTech");
	mhWVP = mFX->GetParameterByName(0, "gWVP");
	mhLightWVP = mFX->GetParameterByName(0, "gLightWVP");
	mhWorldInvTrans = mFX->GetParameterByName(0, "gWorldInvTrans");
	mhMtrl = mFX->GetParameterByName(0, "gMtrl");
	mhLight = mFX->GetParameterByName(0, "gLight");
	mhEyePosW = mFX->GetParameterByName(0, "gEyePosW");
	mhWorld = mFX->GetParameterByName(0, "gWorld");
	mhTex = mFX->GetParameterByName(0, "gTex");
}
