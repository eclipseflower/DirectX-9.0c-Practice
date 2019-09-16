#include "RenderToTexDemoApp.h"
#include "DirectInput.h"

RenderToTexDemoApp::RenderToTexDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP) : D3DApp(hInstance, winCaption, devType, requestedVP)
{
	mAutoGenMips = true;

	if (!CheckDeviceCaps())
	{
		MessageBox(0, "checkDeviceCaps() Failed", 0, 0);
		PostQuitMessage(0);
	}

	gCamera = &mFirstPersonCamera;

	InitAllVertexDeclarations();

	mGfxStats = new GfxStats();
	mSky = new Sky("grassenvmap1024.dds", 10000.0f);

	// Viewport is entire texture.
	D3DVIEWPORT9 vp = { 0, 0, 256, 256, 0.0f, 1.0f };
	mRadarMap = new DrawableTex2D(256, 256, 0, D3DFMT_X8R8G8B8, true, D3DFMT_D24X8, vp, mAutoGenMips);
	HR(gD3dDevice->CreateVertexBuffer(6 * sizeof(VertexPT), D3DUSAGE_WRITEONLY,
		0, D3DPOOL_MANAGED, &mRadarVB, 0));

	// Radar quad takes up quadrant IV.  Note that we specify coordinate directly in
	// normalized device coordinates.  I.e., world, view, projection matrices are all
	// identity.
	VertexPT* v = 0;
	HR(mRadarVB->Lock(0, 0, (void**)&v, 0));
	v[0] = VertexPT(0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[1] = VertexPT(1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[2] = VertexPT(0.0f, -1.0f, 0.0f, 0.0f, 1.0f);
	v[3] = VertexPT(0.0f, -1.0f, 0.0f, 0.0f, 1.0f);
	v[4] = VertexPT(1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[5] = VertexPT(1.0f, -1.0f, 0.0f, 1.0f, 1.0f);
	HR(mRadarVB->Unlock());

	mTerrain = new Terrain(513, 513, 4.0f, 4.0f,
		"coastMountain513.raw",
		"grass.dds",
		"dirt.dds",
		"rock.dds",
		"blend_hm17.dds",
		1.5f, 0.0f);

	D3DXVECTOR3 toSun(1.0f, 1.0f, 1.0f);
	D3DXVec3Normalize(&toSun, &toSun);
	mTerrain->SetDirToSunW(toSun);

	// Initialize camera.
	gCamera->Pos().y = 250.0f;
	gCamera->SetSpeed(50.0f);

	mGfxStats->AddVertices(mTerrain->GetNumVertices());
	mGfxStats->AddTriangles(mTerrain->GetNumTriangles());

	BuildFX();

	OnResetDevice();
}

RenderToTexDemoApp::~RenderToTexDemoApp()
{
	delete mGfxStats;
	delete mTerrain;
	delete mSky;
	delete mRadarMap;
	ReleaseCOM(mRadarVB);
	ReleaseCOM(mRadarFX);

	DestroyAllVertexDeclarations();
}

bool RenderToTexDemoApp::CheckDeviceCaps()
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
	// both.  We use D3DFMT_X8R8G8B8 as the render texture format and D3DFMT_D24X8 as the 
	// render texture depth format.
	D3DDISPLAYMODE mode;
	mD3dObject->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &mode);

	// Windowed.
	if (FAILED(mD3dObject->CheckDeviceFormat(D3DADAPTER_DEFAULT, mDevType, mode.Format,
		D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, D3DFMT_X8R8G8B8)))
		return false;
	if (FAILED(mD3dObject->CheckDepthStencilMatch(D3DADAPTER_DEFAULT, mDevType, mode.Format,
		D3DFMT_X8R8G8B8, D3DFMT_D24X8)))
		return false;

	// Fullscreen.
	if (FAILED(mD3dObject->CheckDeviceFormat(D3DADAPTER_DEFAULT, mDevType, D3DFMT_X8R8G8B8,
		D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, D3DFMT_X8R8G8B8)))
		return false;
	if (FAILED(mD3dObject->CheckDepthStencilMatch(D3DADAPTER_DEFAULT, mDevType, D3DFMT_X8R8G8B8,
		D3DFMT_X8R8G8B8, D3DFMT_D24X8)))
		return false;

	if (caps.Caps2 & D3DCAPS2_CANAUTOGENMIPMAP)
	{
		HRESULT hr = D3D_OK;

		// Windowed.
		hr = mD3dObject->CheckDeviceFormat(D3DADAPTER_DEFAULT,
			D3DDEVTYPE_HAL, mode.Format, D3DUSAGE_AUTOGENMIPMAP,
			D3DRTYPE_TEXTURE, D3DFMT_X8R8G8B8);
		if (hr == D3DOK_NOAUTOGEN)
			mAutoGenMips = false;

		// Fullscreen.
		hr = mD3dObject->CheckDeviceFormat(D3DADAPTER_DEFAULT,
			D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, D3DUSAGE_AUTOGENMIPMAP,
			D3DRTYPE_TEXTURE, D3DFMT_X8R8G8B8);
		if (hr == D3DOK_NOAUTOGEN)
			mAutoGenMips = false;

	}

	return true;
}

void RenderToTexDemoApp::OnLostDevice()
{
	mGfxStats->OnLostDevice();
	mTerrain->OnLostDevice();
	mRadarMap->OnLostDevice();
	mSky->OnLostDevice();
	HR(mRadarFX->OnLostDevice());
}

void RenderToTexDemoApp::OnResetDevice()
{
	mGfxStats->OnResetDevice();
	mTerrain->OnResetDevice();
	mRadarMap->OnResetDevice();
	mSky->OnResetDevice();
	HR(mRadarFX->OnResetDevice());

	// The aspect ratio depends on the backbuffer dimensions, which can 
	// possibly change after a reset.  So rebuild the projection matrix.
	float w = (float)mD3dpp.BackBufferWidth;
	float h = (float)mD3dpp.BackBufferHeight;
	mFirstPersonCamera.SetLens(D3DX_PI * 0.25f, w / h, 1.0f, 2000.0f);
	mBirdsEyeCamera.SetLens(D3DX_PI * 0.25f, w / h, 1.0f, 2000.0f);
}

void RenderToTexDemoApp::UpdateScene(float dt)
{
	mGfxStats->Update(dt);

	gDInput->poll();

	gCamera->Update(dt, mTerrain, 5.5f);
}

void RenderToTexDemoApp::DrawScene()
{
	D3DXVECTOR3 pos(gCamera->Pos().x, gCamera->Pos().y + 1000.0f, gCamera->Pos().z);
	D3DXVECTOR3 up(0.0f, 0.0f, 1.0f);
	mBirdsEyeCamera.LookAt(pos, gCamera->Pos(), up);

	mRadarMap->BeginScene();
	gD3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xff000000, 1.0f, 0);
	gCamera = &mBirdsEyeCamera;
	mTerrain->Draw();
	mRadarMap->EndScene();
	gCamera = &mFirstPersonCamera;

	HR(gD3dDevice->BeginScene());
	mSky->Draw();

	mTerrain->Draw();

	HR(gD3dDevice->SetStreamSource(0, mRadarVB, 0, sizeof(VertexPT)));
	HR(gD3dDevice->SetVertexDeclaration(VertexPT::decl));

	HR(mRadarFX->SetTexture(mhTex, mRadarMap->D3dTex()));
	UINT numPasses = 0;
	HR(mRadarFX->Begin(&numPasses, 0));
	HR(mRadarFX->BeginPass(0));
	HR(gD3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2));
	HR(mRadarFX->EndPass());
	HR(mRadarFX->End());

	mGfxStats->Display();

	HR(gD3dDevice->EndScene());

	// Present the backbuffer.
	HR(gD3dDevice->Present(0, 0, 0, 0));
}

void RenderToTexDemoApp::BuildFX()
{
	// Create the generic Light & Tex FX from a .fx file.
	ID3DXBuffer* errors = 0;
	HR(D3DXCreateEffectFromFile(gD3dDevice, "radar.fx",
		0, 0, 0, 0, &mRadarFX, &errors));
	if (errors)
		MessageBox(0, (char*)errors->GetBufferPointer(), 0, 0);

	// Obtain handles.
	mhTech = mRadarFX->GetTechniqueByName("RadarTech");
	mhTex = mRadarFX->GetParameterByName(0, "gTex");
}
