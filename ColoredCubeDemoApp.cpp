#include "Vertex.h"
#include "DirectInput.h"
#include "ColoredCubeDemoApp.h"

ColoredCubeDemoApp::ColoredCubeDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP) : D3DApp(hInstance, winCaption, devType, requestedVP)
{
	if (!CheckDeviceCaps())
	{
		MessageBox(0, "checkDeviceCaps() Failed", 0, 0);
		PostQuitMessage(0);
	}

	mGfxStats = new GfxStats();

	mCameraRadius = 10.0f;
	mCameraRotationY = 1.2 * D3DX_PI;
	mCameraHeight = 5.0f;

	BuildVertexBuffer();
	BuildIndexBuffer();
	BuildFx();

	OnResetDevice();
	InitAllVertexDeclarations();
}

ColoredCubeDemoApp::~ColoredCubeDemoApp()
{
	delete mGfxStats;
	ReleaseCOM(mVb);
	ReleaseCOM(mIb);
	ReleaseCOM(mFx);
	
	DestroyAllVertexDeclarations();
}

bool ColoredCubeDemoApp::CheckDeviceCaps()
{
	D3DCAPS9 caps;
	HR(gD3dDevice->GetDeviceCaps(&caps));

	if (caps.VertexShaderVersion < D3DVS_VERSION(2, 0))
		return false;

	if (caps.PixelShaderVersion < D3DPS_VERSION(2, 0))
		return false;

	return true;
}

void ColoredCubeDemoApp::OnLostDevice()
{
	mGfxStats->OnLostDevice();
	HR(mFx->OnLostDevice());
}

void ColoredCubeDemoApp::OnResetDevice()
{
	mGfxStats->OnResetDevice();
	HR(mFx->OnResetDevice());
	BuildProjMtx();
}

void ColoredCubeDemoApp::UpdateScene(float dt)
{
	mGfxStats->SetVertexCount(8);
	mGfxStats->SetTriCount(12);
	mGfxStats->Update(dt);

	gDInput->poll();
	if (gDInput->keyDown(DIK_W))
		mCameraHeight += 25.0f * dt;
	if (gDInput->keyDown(DIK_S))
		mCameraHeight -= 25.0f * dt;

	mCameraRotationY += gDInput->mouseDX() / 100.0f;
	mCameraRadius += gDInput->mouseDY() / 25.0f;

	if (fabsf(mCameraRotationY) >= 2.0f * D3DX_PI)
		mCameraRotationY = 0.0f;

	if (mCameraRadius < 5.0f)
		mCameraRadius = 5.0f;

	BuildViewMtx();
}

void ColoredCubeDemoApp::DrawScene()
{
	HR(gD3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffeeeeee, 1.0f, 0));
	HR(gD3dDevice->BeginScene());
	HR(gD3dDevice->SetStreamSource(0, mVb, 0, sizeof(VertexCol)));
	HR(gD3dDevice->SetIndices(mIb));
	HR(gD3dDevice->SetVertexDeclaration(VertexCol::decl));
	HR(mFx->SetTechnique(mhTech));
	HR(mFx->SetMatrix(mhWVP, &(mView*mProj)));
	UINT numPasses = 0;
	HR(mFx->Begin(&numPasses, 0));
	for (UINT i = 0; i < numPasses; ++i)
	{
		HR(mFx->BeginPass(i));
		HR(gD3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 8, 0, 12));
		HR(mFx->EndPass());
	}
	HR(mFx->End());


	mGfxStats->Display();

	HR(gD3dDevice->EndScene());
	HR(gD3dDevice->Present(0, 0, 0, 0));
}

void ColoredCubeDemoApp::BuildVertexBuffer()
{
	HR(gD3dDevice->CreateVertexBuffer(8 * sizeof(VertexCol), D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &mVb, 0));

	VertexCol* v = NULL;
	HR(mVb->Lock(0, 0, (void**)&v, 0));

	v[0] = VertexCol(-1.0f, -1.0f, -1.0f, WHITE);
	v[1] = VertexCol(-1.0f, 1.0f, -1.0f, BLACK);
	v[2] = VertexCol(1.0f, 1.0f, -1.0f, RED);
	v[3] = VertexCol(1.0f, -1.0f, -1.0f, GREEN);
	v[4] = VertexCol(-1.0f, -1.0f, 1.0f, BLUE);
	v[5] = VertexCol(-1.0f, 1.0f, 1.0f, YELLOW);
	v[6] = VertexCol(1.0f, 1.0f, 1.0f, CYAN);
	v[7] = VertexCol(1.0f, -1.0f, 1.0f, MAGENTA);

	HR(mVb->Unlock());
}

void ColoredCubeDemoApp::BuildIndexBuffer()
{
	HR(gD3dDevice->CreateIndexBuffer(36 * sizeof(WORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &mIb, 0));

	WORD* k = 0;

	HR(mIb->Lock(0, 0, (void**)&k, 0));

	// Front face.
	k[0] = 0; k[1] = 1; k[2] = 2;
	k[3] = 0; k[4] = 2; k[5] = 3;

	// Back face.
	k[6] = 4; k[7] = 6; k[8] = 5;
	k[9] = 4; k[10] = 7; k[11] = 6;

	// Left face.
	k[12] = 4; k[13] = 5; k[14] = 1;
	k[15] = 4; k[16] = 1; k[17] = 0;

	// Right face.
	k[18] = 3; k[19] = 2; k[20] = 6;
	k[21] = 3; k[22] = 6; k[23] = 7;

	// Top face.
	k[24] = 1; k[25] = 5; k[26] = 6;
	k[27] = 1; k[28] = 6; k[29] = 2;

	// Bottom face.
	k[30] = 4; k[31] = 0; k[32] = 3;
	k[33] = 4; k[34] = 3; k[35] = 7;

	HR(mIb->Unlock());
}

void ColoredCubeDemoApp::BuildFx()
{
	ID3DXBuffer *errors = NULL;
	HR(D3DXCreateEffectFromFile(gD3dDevice, "color.fx", 0, 0, D3DXSHADER_DEBUG | D3DXSHADER_ENABLE_BACKWARDS_COMPATIBILITY, 0, &mFx, &errors));
	if(errors)
		MessageBox(0, (char*)errors->GetBufferPointer(), 0, 0);
	mhTech = mFx->GetTechniqueByName("ColorTech");
	mhWVP = mFx->GetParameterByName(0, "gWVP");
}

void ColoredCubeDemoApp::BuildViewMtx()
{
	float x = mCameraRadius * cosf(mCameraRotationY);
	float z = mCameraRadius * sinf(mCameraRotationY);
	D3DXVECTOR3 pos(x, mCameraHeight, z);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&mView, &pos, &target, &up);
}

void ColoredCubeDemoApp::BuildProjMtx()
{
	float w = (float)mD3dpp.BackBufferWidth;
	float h = (float)mD3dpp.BackBufferHeight;
	D3DXMatrixPerspectiveFovLH(&mProj, D3DX_PI * 0.25f, w / h, 1.0f, 5000.0f);
}
