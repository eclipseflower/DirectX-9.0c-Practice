#include "Vertex.h"
#include "WalkTerrainDemoApp.h"
#include "DirectInput.h"
#include "Camera.h"

WalkTerrainDemoApp::WalkTerrainDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP) : D3DApp(hInstance, winCaption, devType, requestedVP)
{
	if (!CheckDeviceCaps())
	{
		MessageBox(0, "checkDeviceCaps() Failed", 0, 0);
		PostQuitMessage(0);
	}

	InitAllVertexDeclarations();

	mGfxStats = new GfxStats();

	// World space units are meters.  So (256*10.0f)x(256*10.0f) is (2.56)^2 square
	// kilometers.
	mTerrain = new Terrain(257, 257, 10.0f, 10.0f,
		"heightmap17_257.raw",
		"grass.dds",
		"dirt.dds",
		"stone.dds",
		"blend_hm17.dds",
		3.0f, 0.0f);

	mTerrain->SetDirToSunW(D3DXVECTOR3(0.0f, 1.0f, 0.0f));

	mGfxStats->AddVertices(mTerrain->GetNumVertices());
	mGfxStats->AddTriangles(mTerrain->GetNumTriangles());

	OnResetDevice();
}

WalkTerrainDemoApp::~WalkTerrainDemoApp()
{
	delete mGfxStats;
	delete mTerrain;

	DestroyAllVertexDeclarations();
}

bool WalkTerrainDemoApp::CheckDeviceCaps()
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

void WalkTerrainDemoApp::OnLostDevice()
{
	mGfxStats->OnLostDevice();
	mTerrain->OnLostDevice();
}

void WalkTerrainDemoApp::OnResetDevice()
{
	mGfxStats->OnResetDevice();
	mTerrain->OnResetDevice();


	// The aspect ratio depends on the backbuffer dimensions, which can 
	// possibly change after a reset.  So rebuild the projection matrix.
	float w = (float)mD3dpp.BackBufferWidth;
	float h = (float)mD3dpp.BackBufferHeight;
	gCamera->SetLens(D3DX_PI * 0.25f, w / h, 0.01f, 5000.0f);
}

void WalkTerrainDemoApp::UpdateScene(float dt)
{
	mGfxStats->Update(dt);

	gDInput->poll();

	gCamera->Update(dt, mTerrain, 2.5f);
}

void WalkTerrainDemoApp::DrawScene()
{
	HR(gD3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffeeeeee, 1.0f, 0));

	HR(gD3dDevice->BeginScene());

	mTerrain->Draw();

	mGfxStats->Display();

	HR(gD3dDevice->EndScene());

	// Present the backbuffer.
	HR(gD3dDevice->Present(0, 0, 0, 0));
}
