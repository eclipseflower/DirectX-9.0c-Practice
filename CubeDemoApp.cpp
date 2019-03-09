#include "CubeDemoApp.h"
#include "Vertex.h"
#include "DirectInput.h"

CubeDemoApp::CubeDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP) : D3DApp(hInstance, winCaption, devType, requestedVP)
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

	OnResetDevice();

	InitAllVertexDeclarations();
}

CubeDemoApp::~CubeDemoApp()
{
	delete mGfxStats;
	ReleaseCOM(mVb);
	ReleaseCOM(mIb);
	DestroyAllVertexDeclarations();
}

bool CubeDemoApp::CheckDeviceCaps()
{
	return true;
}

void CubeDemoApp::OnLostDevice()
{
	mGfxStats->OnLostDevice();
}

void CubeDemoApp::OnResetDevice()
{
	mGfxStats->OnResetDevice();
	BuildProjMtx();
}

void CubeDemoApp::UpdateScene(float dt)
{
	mGfxStats->SetVertexCount(8);
	mGfxStats->SetTriCount(12);
	mGfxStats->Update(dt);

	gDInput->poll();

	if (gDInput->keyDown(DIK_W))
		mCameraHeight += 25.0f * dt;
	if (gDInput->keyDown(DIK_S))
		mCameraHeight -= 25.0f * dt;

	mCameraRotationY += gDInput->mouseDX() * 0.01f;
	mCameraRadius += gDInput->mouseDY() * 0.04f;

	if (fabsf(mCameraRotationY) >= 2.0f * D3DX_PI)
		mCameraRotationY = 0.0f;

	if (mCameraRadius < 5.0f)
		mCameraRadius = 5.0f;

	BuildViewMtx();
}

void CubeDemoApp::DrawScene()
{
	HR(gD3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0));
	HR(gD3dDevice->BeginScene());
	HR(gD3dDevice->SetStreamSource(0, mVb, 0, sizeof(VertexPos)));
	HR(gD3dDevice->SetIndices(mIb));
	HR(gD3dDevice->SetVertexDeclaration(VertexPos::decl));

	D3DXMATRIX W;
	D3DXMatrixIdentity(&W);
	HR(gD3dDevice->SetTransform(D3DTS_WORLD, &W));
	HR(gD3dDevice->SetTransform(D3DTS_VIEW, &mView));
	HR(gD3dDevice->SetTransform(D3DTS_PROJECTION, &mProj));
	HR(gD3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME));
	HR(gD3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 8, 0, 12));
	mGfxStats->Display();
	HR(gD3dDevice->EndScene());
	HR(gD3dDevice->Present(0, 0, 0, 0));
}

void CubeDemoApp::BuildVertexBuffer()
{
	HR(gD3dDevice->CreateVertexBuffer(8 * sizeof(VertexPos), D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &mVb, 0));
	VertexPos *v = NULL;
	HR(mVb->Lock(0, 0, (void **)&v, 0));
	v[0] = VertexPos(-1.0f, -1.0f, -1.0f);
	v[1] = VertexPos(-1.0f, 1.0f, -1.0f);
	v[2] = VertexPos(1.0f, 1.0f, -1.0f);
	v[3] = VertexPos(1.0f, -1.0f, -1.0f);
	v[4] = VertexPos(-1.0f, -1.0f, 1.0f);
	v[5] = VertexPos(-1.0f, 1.0f, 1.0f);
	v[6] = VertexPos(1.0f, 1.0f, 1.0f);
	v[7] = VertexPos(1.0f, -1.0f, 1.0f);
	HR(mVb->Unlock());
}

void CubeDemoApp::BuildIndexBuffer()
{
	HR(gD3dDevice->CreateIndexBuffer(36 * sizeof(WORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &mIb, 0));
	WORD *k = NULL;
	HR(mIb->Lock(0, 0, (void **)&k, 0));
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

void CubeDemoApp::BuildViewMtx()
{
	float x = mCameraRadius * cosf(mCameraRotationY);
	float z = mCameraRadius * cosf(mCameraRotationY);
	D3DXVECTOR3 pos(x, mCameraHeight, z);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&mView, &pos, &target, &up);
}

void CubeDemoApp::BuildProjMtx()
{
	float w = (float)mD3dpp.BackBufferWidth;
	float h = (float)mD3dpp.BackBufferHeight;
	D3DXMatrixPerspectiveFovLH(&mProj, D3DX_PI * 0.25f, w / h, 1.0f, 5000.0f);
}
