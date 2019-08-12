#include "GunDemoApp.h"
#include "Terrain.h"
#include "DirectInput.h"

GunDemoApp::GunDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP) : D3DApp(hInstance, winCaption, devType, requestedVP)
{
	if (!CheckDeviceCaps())
	{
		MessageBox(0, "checkDeviceCaps() Failed", 0, 0);
		PostQuitMessage(0);
	}

	InitAllVertexDeclarations();

	mGfxStats = new GfxStats();

	// World space units are meters.  
	mTerrain = new Terrain(257, 257, 2.0f, 2.0f,
		"heightmap1_257.raw",
		"mud.dds",
		"stone.dds",
		"snow.dds",
		"blend_hm1.dds",
		0.4f, 0.0f);

	D3DXVECTOR3 toSun(1.0f, 1.0f, 1.0f);
	D3DXVec3Normalize(&toSun, &toSun);
	mTerrain->SetDirToSunW(toSun);

	// Initialize camera.
	gCamera->Pos() = D3DXVECTOR3(55.0f, 50.0f, 25.0f);
	gCamera->SetSpeed(40.0f);

	// Initialize the particle system.
	D3DXMATRIX psysWorld;
	D3DXMatrixIdentity(&psysWorld);

	// Gun bullets spread over a long distance and there is not
	// a high bullet count, so not really worth trying to cull.
	AABB psysBox;
	psysBox.maxPt = D3DXVECTOR3(INFINITY, INFINITY, INFINITY);
	psysBox.minPt = D3DXVECTOR3(-INFINITY, -INFINITY, -INFINITY);

	// Accelerate due to gravity.  However, since the bullets travel at 
	// such a high velocity, the effect of gravity of not really observed.
	mPSys = new Gun("gun.fx", "GunTech", "bolt.dds",
		D3DXVECTOR3(0, -9.8f, 0.0f), psysBox, 100, -1.0f);
	mPSys->SetWorldMtx(psysWorld);

	mGfxStats->AddVertices(mTerrain->GetNumVertices());
	mGfxStats->AddTriangles(mTerrain->GetNumTriangles());

	OnResetDevice();
}

GunDemoApp::~GunDemoApp()
{
	delete mGfxStats;
	delete mTerrain;
	delete mPSys;

	DestroyAllVertexDeclarations();
}

bool GunDemoApp::CheckDeviceCaps()
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

void GunDemoApp::OnLostDevice()
{
	mGfxStats->OnLostDevice();
	mTerrain->OnLostDevice();
	mPSys->OnLostDevice();
}

void GunDemoApp::OnResetDevice()
{
	mGfxStats->OnResetDevice();
	mTerrain->OnResetDevice();
	mPSys->OnResetDevice();


	// The aspect ratio depends on the backbuffer dimensions, which can 
	// possibly change after a reset.  So rebuild the projection matrix.
	float w = (float)mD3dpp.BackBufferWidth;
	float h = (float)mD3dpp.BackBufferHeight;
	gCamera->SetLens(D3DX_PI * 0.25f, w / h, 0.01f, 5000.0f);
}

void GunDemoApp::UpdateScene(float dt)
{
	mGfxStats->Update(dt);

	gDInput->poll();

	gCamera->Update(dt, 0, 0);

	mPSys->Update(dt);

	// Can only fire once every tenth of a second.
	static float delay = 0.0f;
	if (gDInput->keyDown(DIK_SPACE) && delay <= 0.0f)
	{
		delay = 0.1f;
		mPSys->AddParticle();
	}
	delay -= dt;
}

void GunDemoApp::DrawScene()
{
	// Clear the backbuffer and depth buffer.
	HR(gD3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xff666666, 1.0f, 0));

	HR(gD3dDevice->BeginScene());

	mTerrain->Draw();
	mPSys->Draw();

	mGfxStats->Display();

	HR(gD3dDevice->EndScene());

	// Present the backbuffer.
	HR(gD3dDevice->Present(0, 0, 0, 0));
}
