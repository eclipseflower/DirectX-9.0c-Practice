#include "DisplacementMapDemoApp.h"
#include "Vertex.h"
#include "Camera.h"
#include "DirectInput.h"

DisplacementMapDemoApp::DisplacementMapDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP) : D3DApp(hInstance, winCaption, devType, requestedVP)
{
	if (!CheckDeviceCaps())
	{
		MessageBox(0, "checkDeviceCaps() Failed", 0, 0);
		PostQuitMessage(0);
	}

	InitAllVertexDeclarations();

	mLight.dirW = D3DXVECTOR3(0.0f, -1.0f, -3.0f);
	D3DXVec3Normalize(&mLight.dirW, &mLight.dirW);
	mLight.ambient = D3DXCOLOR(0.3f, 0.3f, 0.3f, 1.0f);
	mLight.diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	mLight.spec = D3DXCOLOR(0.7f, 0.7f, 0.7f, 1.0f);

	mGfxStats = new GfxStats();
	mSky = new Sky("grassenvmap1024.dds", 10000.0f);

	D3DXMATRIX waterWorld;
	D3DXMatrixIdentity(&waterWorld);

	Mtrl waterMtrl;
	waterMtrl.ambient = D3DXCOLOR(0.4f, 0.4f, 0.7f, 0.0f);
	waterMtrl.diffuse = D3DXCOLOR(0.4f, 0.4f, 0.7f, 1.0f);
	waterMtrl.spec = 0.8f*_WHITE;
	waterMtrl.specPower = 128.0f;

	WaterDMap::InitInfo waterInitInfo;
	waterInitInfo.dirLight = mLight;
	waterInitInfo.mtrl = waterMtrl;
	waterInitInfo.fxFilename = "waterdmap.fx";
	waterInitInfo.vertRows = 128;
	waterInitInfo.vertCols = 128;
	waterInitInfo.dx = 0.25f;
	waterInitInfo.dz = 0.25f;
	waterInitInfo.waveMapFilename0 = "wave0.dds";
	waterInitInfo.waveMapFilename1 = "wave1.dds";
	waterInitInfo.dmapFilename0 = "waterdmap0.dds";
	waterInitInfo.dmapFilename1 = "waterdmap1.dds";
	waterInitInfo.waveNMapVelocity0 = D3DXVECTOR2(0.05f, 0.07f);
	waterInitInfo.waveNMapVelocity1 = D3DXVECTOR2(-0.01f, 0.13f);
	waterInitInfo.waveDMapVelocity0 = D3DXVECTOR2(0.012f, 0.015f);
	waterInitInfo.waveDMapVelocity1 = D3DXVECTOR2(0.014f, 0.05f);
	waterInitInfo.scaleHeights = D3DXVECTOR2(0.7f, 1.1f);
	waterInitInfo.texScale = 8.0f;
	waterInitInfo.toWorld = waterWorld;

	mWater = new WaterDMap(waterInitInfo);

	// Initialize camera.
	gCamera->Pos().y = 1.0f;
	gCamera->Pos().z = -15.0f;
	gCamera->SetSpeed(5.0f);

	mGfxStats->AddVertices(mWater->GetNumVertices());
	mGfxStats->AddTriangles(mWater->GetNumTriangles());

	mGfxStats->AddVertices(mSky->GetNumVertices());
	mGfxStats->AddTriangles(mSky->GetNumTriangles());

	OnResetDevice();
}

DisplacementMapDemoApp::~DisplacementMapDemoApp()
{
	delete mGfxStats;
	delete mSky;
	delete mWater;

	DestroyAllVertexDeclarations();
}

bool DisplacementMapDemoApp::CheckDeviceCaps()
{
	D3DCAPS9 caps;
	HR(gD3dDevice->GetDeviceCaps(&caps));

	// Check for vertex shader version 3.0 support.
	if (caps.VertexShaderVersion < D3DVS_VERSION(3, 0))
		return false;

	// Check for pixel shader version 3.0 support.
	if (caps.PixelShaderVersion < D3DPS_VERSION(3, 0))
		return false;

	return true;
}

void DisplacementMapDemoApp::OnLostDevice()
{
	mGfxStats->OnLostDevice();
	mSky->OnLostDevice();
	mWater->OnLostDevice();
}

void DisplacementMapDemoApp::OnResetDevice()
{
	mGfxStats->OnResetDevice();
	mSky->OnResetDevice();
	mWater->OnResetDevice();

	// The aspect ratio depends on the backbuffer dimensions, which can 
	// possibly change after a reset.  So rebuild the projection matrix.
	float w = (float)mD3dpp.BackBufferWidth;
	float h = (float)mD3dpp.BackBufferHeight;
	gCamera->SetLens(D3DX_PI * 0.25f, w / h, 1.0f, 1000.0f);
}

void DisplacementMapDemoApp::UpdateScene(float dt)
{
	mGfxStats->Update(dt);

	gDInput->poll();

	// Prevent camera from getting too close to water
	if (gCamera->Pos().y < 2.0f)
		gCamera->Pos().y = 2.0f;

	gCamera->Update(dt, 0, 0);


	mWater->Update(dt);
}

void DisplacementMapDemoApp::DrawScene()
{
	HR(gD3dDevice->BeginScene());

	mSky->Draw();

	mWater->Draw();

	mGfxStats->Display();

	HR(gD3dDevice->EndScene());

	// Present the backbuffer.
	HR(gD3dDevice->Present(0, 0, 0, 0));
}
