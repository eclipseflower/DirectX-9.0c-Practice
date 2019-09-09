#include "EnvMapDemoApp.h"
#include "Vertex.h"
#include "Camera.h"
#include "DirectInput.h"

EnvMapDemoApp::EnvMapDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP) : D3DApp(hInstance, winCaption, devType, requestedVP)
{
	if (!CheckDeviceCaps())
	{
		MessageBox(0, "checkDeviceCaps() Failed", 0, 0);
		PostQuitMessage(0);
	}

	InitAllVertexDeclarations();

	mGfxStats = new GfxStats();
	mSky = new Sky("grassenvmap1024.dds", 10000.0f);

	mLight.dirW = D3DXVECTOR3(0.0f, 1.0f, 2.0f);
	D3DXVec3Normalize(&mLight.dirW, &mLight.dirW);
	mLight.ambient = D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f);
	mLight.diffuse = D3DXCOLOR(0.8f, 0.8f, 0.8f, 1.0f);
	mLight.spec = D3DXCOLOR(0.8f, 0.8f, 0.8f, 1.0f);

	LoadXFile("skullocc.x", &mSceneMesh, mSceneMtrls, mSceneTextures);
	D3DXMatrixIdentity(&mSceneWorld);

	HR(D3DXCreateTextureFromFile(gD3dDevice, "whitetex.dds", &mWhiteTex));

	// Initialize camera.
	gCamera->Pos().y = 3.0f;
	gCamera->Pos().z = -10.0f;
	gCamera->SetSpeed(5.0f);

	mGfxStats->AddVertices(mSceneMesh->GetNumVertices());
	mGfxStats->AddTriangles(mSceneMesh->GetNumFaces());

	mGfxStats->AddVertices(mSky->GetNumVertices());
	mGfxStats->AddTriangles(mSky->GetNumTriangles());

	BuildFX();

	OnResetDevice();
}

EnvMapDemoApp::~EnvMapDemoApp()
{
	delete mGfxStats;
	delete mSky;

	ReleaseCOM(mFX);

	ReleaseCOM(mSceneMesh);
	for (UINT i = 0; i < mSceneTextures.size(); ++i)
		ReleaseCOM(mSceneTextures[i]);

	ReleaseCOM(mWhiteTex);

	DestroyAllVertexDeclarations();
}

bool EnvMapDemoApp::CheckDeviceCaps()
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

void EnvMapDemoApp::OnLostDevice()
{
	mGfxStats->OnLostDevice();
	mSky->OnLostDevice();
	HR(mFX->OnLostDevice());
}

void EnvMapDemoApp::OnResetDevice()
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

void EnvMapDemoApp::UpdateScene(float dt)
{
	mGfxStats->Update(dt);

	gDInput->poll();

	gCamera->Update(dt, 0, 0);
}

void EnvMapDemoApp::DrawScene()
{
	HR(gD3dDevice->BeginScene());

	// Draw sky first--this also replaces our gd3dDevice->Clear call.
	mSky->Draw();

	HR(mFX->SetValue(mhLight, &mLight, sizeof(DirLight)));
	HR(mFX->SetMatrix(mhWVP, &(mSceneWorld*gCamera->ViewProj())));
	HR(mFX->SetValue(mhEyePosW, &gCamera->Pos(), sizeof(D3DXVECTOR3)));

	UINT numPasses = 0;
	HR(mFX->Begin(&numPasses, 0));
	HR(mFX->BeginPass(0));

	for (UINT j = 0; j < mSceneMtrls.size(); ++j)
	{
		HR(mFX->SetValue(mhMtrl, &mSceneMtrls[j], sizeof(Mtrl)));

		// If there is a texture, then use.
		if (mSceneTextures[j] != 0)
		{
			HR(mFX->SetTexture(mhTex, mSceneTextures[j]));
		}

		// But if not, then set a pure white texture.  When the texture color
		// is multiplied by the color from lighting, it is like multiplying by
		// 1 and won't change the color from lighting.
		else
		{
			HR(mFX->SetTexture(mhTex, mWhiteTex));
		}

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

void EnvMapDemoApp::BuildFX()
{
	ID3DXBuffer* errors = 0;
	HR(D3DXCreateEffectFromFile(gD3dDevice, "envmap.fx",
		0, 0, D3DXSHADER_DEBUG, 0, &mFX, &errors));
	if (errors)
		MessageBox(0, (char*)errors->GetBufferPointer(), 0, 0);

	// Obtain handles.
	mhTech = mFX->GetTechniqueByName("EnvMapTech");
	mhWVP = mFX->GetParameterByName(0, "gWVP");
	mhWorldInvTrans = mFX->GetParameterByName(0, "gWorldInvTrans");
	mhMtrl = mFX->GetParameterByName(0, "gMtrl");
	mhLight = mFX->GetParameterByName(0, "gLight");
	mhEyePosW = mFX->GetParameterByName(0, "gEyePosW");
	mhWorld = mFX->GetParameterByName(0, "gWorld");
	mhTex = mFX->GetParameterByName(0, "gTex");
	mhEnvMap = mFX->GetParameterByName(0, "gEnvMap");

	// Set parameters that do not vary:

	// World is the identity, so inverse-transpose also identity.
	HR(mFX->SetMatrix(mhWorldInvTrans, &mSceneWorld));
	HR(mFX->SetMatrix(mhWorld, &mSceneWorld));

	HR(mFX->SetTexture(mhEnvMap, mSky->GetEnvMap()));
	HR(mFX->SetTechnique(mhTech));
}
