#include "CullingDemoApp.h"
#include "Camera.h"
#include "DirectInput.h"

CullingDemoApp::CullingDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP) : D3DApp(hInstance, winCaption, devType, requestedVP)
{
	if (!CheckDeviceCaps())
	{
		MessageBox(0, "checkDeviceCaps() Failed", 0, 0);
		PostQuitMessage(0);
	}

	InitAllVertexDeclarations();

	mGfxStats = new GfxStats();

	mTerrain = new Terrain(513, 513, 4.0f, 4.0f,
		"coastMountain513.raw",
		"grass.dds",
		"dirt.dds",
		"rock.dds",
		"blend_coastal.dds",
		1.5f, 0.0f);

	D3DXVECTOR3 toSun(1.0f, 1.0f, 1.0f);
	D3DXVec3Normalize(&toSun, &toSun);
	mTerrain->SetDirToSunW(toSun);

	// Initialize camera.
	gCamera->Pos().y = 250.0f;
	gCamera->SetSpeed(50.0f);

	mGfxStats->AddVertices(mTerrain->GetNumVertices());
	mGfxStats->AddTriangles(mTerrain->GetNumTriangles());

	OnResetDevice();
}

CullingDemoApp::~CullingDemoApp()
{
	delete mGfxStats;
	delete mTerrain;

	DestroyAllVertexDeclarations();
}

bool CullingDemoApp::CheckDeviceCaps()
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

void CullingDemoApp::OnLostDevice()
{
	mGfxStats->OnLostDevice();
	mTerrain->OnLostDevice();
}

void CullingDemoApp::OnResetDevice()
{
	mGfxStats->OnResetDevice();
	mTerrain->OnResetDevice();


	// The aspect ratio depends on the backbuffer dimensions, which can 
	// possibly change after a reset.  So rebuild the projection matrix.
	float w = (float)mD3dpp.BackBufferWidth;
	float h = (float)mD3dpp.BackBufferHeight;
	gCamera->SetLens(D3DX_PI * 0.25f, w / h, 1.0f, 2000.0f);
}

void CullingDemoApp::UpdateScene(float dt)
{
	mGfxStats->Update(dt);
	gDInput->poll();
	gCamera->Update(dt, mTerrain, 2.5f);
}

void CullingDemoApp::DrawScene()
{
	// Clear the backbuffer and depth buffer.
	HR(gD3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffeeeeee, 1.0f, 0));

	HR(gD3dDevice->BeginScene());

	mTerrain->Draw();

	mGfxStats->Display();

	HR(gD3dDevice->EndScene());

	// Present the backbuffer.
	HR(gD3dDevice->Present(0, 0, 0, 0));
}
