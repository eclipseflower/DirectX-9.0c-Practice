#include "TriPickDemoApp.h"
#include "Vertex.h"
#include "Camera.h"
#include "DirectInput.h"

TriPickDemoApp::TriPickDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP) : D3DApp(hInstance, winCaption, devType, requestedVP)
{
	if (!CheckDeviceCaps())
	{
		MessageBox(0, "checkDeviceCaps() Failed", 0, 0);
		PostQuitMessage(0);
	}

	InitAllVertexDeclarations();

	mGfxStats = new GfxStats();

	// Load the mesh data.
	LoadXFile("car.x", &mMesh, mMeshMtrls, mMeshTextures);

	// Initialize camera.
	gCamera->Pos() = D3DXVECTOR3(-0.0f, 2.0f, -15.0f);
	gCamera->SetSpeed(40.0f);

	// Load the default texture.
	HR(D3DXCreateTextureFromFile(gD3dDevice, "whitetex.dds", &mWhiteTex));

	// Init a light.
	mLight.dirW = D3DXVECTOR3(0.707f, 0.0f, 0.707f);
	D3DXVec3Normalize(&mLight.dirW, &mLight.dirW);
	mLight.ambient = D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f);
	mLight.diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	mLight.spec = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

	BuildFX();

	mGfxStats->AddTriangles(mMesh->GetNumFaces());
	mGfxStats->AddVertices(mMesh->GetNumVertices());

	OnResetDevice();
}

TriPickDemoApp::~TriPickDemoApp()
{
	delete mGfxStats;
	ReleaseCOM(mWhiteTex);
	ReleaseCOM(mFX);

	ReleaseCOM(mMesh);
	for (UINT i = 0; i < mMeshTextures.size(); ++i)
		ReleaseCOM(mMeshTextures[i]);

	DestroyAllVertexDeclarations();
}

bool TriPickDemoApp::CheckDeviceCaps()
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

void TriPickDemoApp::OnLostDevice()
{
	mGfxStats->OnLostDevice();
	HR(mFX->OnLostDevice());
}

void TriPickDemoApp::OnResetDevice()
{
	mGfxStats->OnResetDevice();
	HR(mFX->OnResetDevice());

	// The aspect ratio depends on the backbuffer dimensions, which can 
	// possibly change after a reset.  So rebuild the projection matrix.
	float w = (float)mD3dpp.BackBufferWidth;
	float h = (float)mD3dpp.BackBufferHeight;
	gCamera->SetLens(D3DX_PI * 0.25f, w / h, 0.01f, 5000.0f);
}

void TriPickDemoApp::UpdateScene(float dt)
{
	mGfxStats->Update(dt);

	gDInput->poll();

	gCamera->Update(dt, 0, 0);
}

void TriPickDemoApp::DrawScene()
{
	HR(gD3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0));

	HR(gD3dDevice->BeginScene());

	HR(mFX->SetValue(mhEyePos, &gCamera->Pos(), sizeof(D3DXVECTOR3)));
	HR(mFX->SetTechnique(mhTech));
	UINT numPasses = 0;
	HR(mFX->Begin(&numPasses, 0));
	HR(mFX->BeginPass(0));

	// Specify mesh directly in world space.
	D3DXMATRIX toWorld;
	D3DXMatrixIdentity(&toWorld);

	// Set FX parameters.
	HR(mFX->SetMatrix(mhWVP, &(toWorld*gCamera->ViewProj())));
	D3DXMATRIX worldInvTrans;
	D3DXMatrixInverse(&worldInvTrans, 0, &toWorld);
	D3DXMatrixTranspose(&worldInvTrans, &worldInvTrans);
	HR(mFX->SetMatrix(mhWorldInvTrans, &worldInvTrans));
	HR(mFX->SetMatrix(mhWorld, &toWorld));

	// Draw the car in wireframe mode.
	HR(gD3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME));
	for (UINT j = 0; j < mMeshMtrls.size(); ++j)
	{
		HR(mFX->SetValue(mhMtrl, &mMeshMtrls[j], sizeof(Mtrl)));

		// If there is a texture, then use.
		if (mMeshTextures[j] != 0)
		{
			HR(mFX->SetTexture(mhTex, mMeshTextures[j]));
		}

		// But if not, then set a pure white texture.  When the texture color
		// is multiplied by the color from lighting, it is like multiplying by
		// 1 and won't change the color from lighting.
		else
		{
			HR(mFX->SetTexture(mhTex, mWhiteTex));
		}

		HR(mFX->CommitChanges());
		HR(mMesh->DrawSubset(j));
	}

	// Switch back to solid mode.
	HR(gD3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID));

	// Did we pick anything?
	D3DXVECTOR3 originW(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 dirW(0.0f, 0.0f, 0.0f);
	if (gDInput->mouseButtonDown(0))
	{
		GetWorldPickingRay(originW, dirW);

		BOOL hit = 0;
		DWORD faceIndex = -1;
		float u = 0.0f;
		float v = 0.0f;
		float dist = 0.0f;
		ID3DXBuffer* allhits = 0;
		DWORD numHits = 0;
		HR(D3DXIntersect(mMesh, &originW, &dirW, &hit,
			&faceIndex, &u, &v, &dist, &allhits, &numHits));
		ReleaseCOM(allhits);

		// We hit anything?
		if (hit)
		{
			// Yes, draw the picked triangle in solid mode.
			IDirect3DVertexBuffer9* vb = 0;
			IDirect3DIndexBuffer9* ib = 0;
			HR(mMesh->GetVertexBuffer(&vb));
			HR(mMesh->GetIndexBuffer(&ib));

			HR(gD3dDevice->SetIndices(ib));
			HR(gD3dDevice->SetVertexDeclaration(VertexPNT::decl));
			HR(gD3dDevice->SetStreamSource(0, vb, 0, sizeof(VertexPNT)));

			// faceIndex identifies the picked triangle to draw.
			HR(gD3dDevice->DrawIndexedPrimitive(
				D3DPT_TRIANGLELIST, 0, 0, mMesh->GetNumVertices(), faceIndex * 3, 1))

				ReleaseCOM(vb);
			ReleaseCOM(ib);
		}
	}

	HR(mFX->EndPass());
	HR(mFX->End());

	mGfxStats->Display();

	HR(gD3dDevice->EndScene());

	// Present the backbuffer.
	HR(gD3dDevice->Present(0, 0, 0, 0));
}

void TriPickDemoApp::InitAsteroids()
{
}

void TriPickDemoApp::BuildFX()
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

void TriPickDemoApp::GetWorldPickingRay(D3DXVECTOR3 & originW, D3DXVECTOR3 & dirW)
{
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
