#include "Vertex.h"
#include "DirectInput.h"
#include "AmbientDiffuseSpecularDemoApp.h"


AmbientDiffuseSpecularDemoApp::AmbientDiffuseSpecularDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP) : D3DApp(hInstance, winCaption, devType, requestedVP)
{
	if (!CheckDeviceCaps())
	{
		MessageBox(0, "checkDeviceCaps() Failed", 0, 0);
		PostQuitMessage(0);
	}

	mGfxStats = new GfxStats();

	mCameraRadius = 6.0f;
	mCameraRotationY = 1.2 * D3DX_PI;
	mCameraHeight = 3.0f;

	mLightVecW = D3DXVECTOR3(0.0, 0.0f, -1.0f);
	mDiffuseMtrl = D3DXCOLOR(0.0f, 0.0f, 1.0f, 1.0f);
	mDiffuseLight = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	mAmbientMtrl = D3DXCOLOR(0.0f, 0.0f, 1.0f, 1.0f);
	mAmbientLight = D3DXCOLOR(0.4f, 0.4f, 0.4f, 1.0f);
	mSpecularMtrl = D3DXCOLOR(0.8f, 0.8f, 0.8f, 1.0f);
	mSpecularLight = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	mSpecularPower = 8.0f;

	D3DXMatrixIdentity(&mWorld);

	HR(D3DXCreateTeapot(gD3dDevice, &mTeapot, 0));
	BuildFx();

	OnResetDevice();

	InitAllVertexDeclarations();
}

AmbientDiffuseSpecularDemoApp::~AmbientDiffuseSpecularDemoApp()
{
	delete mGfxStats;
	ReleaseCOM(mTeapot);
	ReleaseCOM(mFx);

	DestroyAllVertexDeclarations();
}

bool AmbientDiffuseSpecularDemoApp::CheckDeviceCaps()
{
	D3DCAPS9 caps;
	HR(gD3dDevice->GetDeviceCaps(&caps));

	if (caps.VertexShaderVersion < D3DVS_VERSION(2, 0))
		return false;

	if (caps.PixelShaderVersion < D3DPS_VERSION(2, 0))
		return false;

	return true;
}

void AmbientDiffuseSpecularDemoApp::OnLostDevice()
{
	mGfxStats->OnLostDevice();
	HR(mFx->OnLostDevice());
}

void AmbientDiffuseSpecularDemoApp::OnResetDevice()
{
	mGfxStats->OnResetDevice();
	HR(mFx->OnResetDevice());
	BuildProjMtx();
}

void AmbientDiffuseSpecularDemoApp::UpdateScene(float dt)
{
	mGfxStats->SetVertexCount(mTeapot->GetNumVertices());
	mGfxStats->SetTriCount(mTeapot->GetNumFaces());
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

void AmbientDiffuseSpecularDemoApp::DrawScene()
{
	HR(gD3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffeeeeee, 1.0f, 0));
	HR(gD3dDevice->BeginScene());

	HR(mFx->SetTechnique(mhTech));
	HR(mFx->SetMatrix(mhWVP, &(mWorld * mView * mProj)));
	D3DXMATRIX worldInverseTranspose;
	D3DXMatrixInverse(&worldInverseTranspose, 0, &mWorld);
	D3DXMatrixTranspose(&worldInverseTranspose, &worldInverseTranspose);
	HR(mFx->SetMatrix(mhWorldInverseTranspose, &worldInverseTranspose));
	HR(mFx->SetValue(mhLightVecW, &mLightVecW, sizeof(D3DXVECTOR3)));
	HR(mFx->SetValue(mhDiffuseMtrl, &mDiffuseMtrl, sizeof(D3DXCOLOR)));
	HR(mFx->SetValue(mhDiffuseLight, &mDiffuseLight, sizeof(D3DXCOLOR)));
	HR(mFx->SetValue(mhAmbientMtrl, &mAmbientMtrl, sizeof(D3DXCOLOR)));
	HR(mFx->SetValue(mhAmbientLight, &mAmbientLight, sizeof(D3DXCOLOR)));
	HR(mFx->SetValue(mhSpecularLight, &mSpecularLight, sizeof(D3DXCOLOR)));
	HR(mFx->SetValue(mhSpecularMtrl, &mSpecularMtrl, sizeof(D3DXCOLOR)));
	HR(mFx->SetFloat(mhSpecularPower, mSpecularPower));
	HR(mFx->SetMatrix(mhWorld, &mWorld));

	UINT numPass = 0;
	HR(mFx->Begin(&numPass, 0));
	for (UINT i = 0; i < numPass; ++i)
	{
		HR(mFx->BeginPass(i));
		HR(mTeapot->DrawSubset(0));
		HR(mFx->EndPass());
	}
	HR(mFx->End());

	mGfxStats->Display();

	HR(gD3dDevice->EndScene());
	HR(gD3dDevice->Present(0, 0, 0, 0));
}

void AmbientDiffuseSpecularDemoApp::BuildFx()
{
	ID3DXBuffer* errors = 0;
	HR(D3DXCreateEffectFromFile(gD3dDevice, "ambientdiffusespec.fx", 0, 0, D3DXSHADER_DEBUG | D3DXSHADER_ENABLE_BACKWARDS_COMPATIBILITY, 0, &mFx, &errors));
	if (errors)
		MessageBox(0, (char*)errors->GetBufferPointer(), 0, 0);

	mhTech = mFx->GetTechniqueByName("AmbientDiffuseSpecTech");
	mhWVP = mFx->GetParameterByName(0, "gWVP");
	mhWorldInverseTranspose = mFx->GetParameterByName(0, "gWorldInverseTranspose");
	mhLightVecW = mFx->GetParameterByName(0, "gLightVecW");
	mhDiffuseMtrl = mFx->GetParameterByName(0, "gDiffuseMtrl");
	mhDiffuseLight = mFx->GetParameterByName(0, "gDiffuseLight");
	mhAmbientMtrl = mFx->GetParameterByName(0, "gAmbientMtrl");
	mhAmbientLight = mFx->GetParameterByName(0, "gAmbientLight");
	mhSpecularMtrl = mFx->GetParameterByName(0, "gSpecularMtrl");
	mhSpecularLight = mFx->GetParameterByName(0, "gSpecularLight");
	mhSpecularPower = mFx->GetParameterByName(0, "gSpecularPower");
	mhEyePos = mFx->GetParameterByName(0, "gEyePosW");
	mhWorld = mFx->GetParameterByName(0, "gWorld");
}

void AmbientDiffuseSpecularDemoApp::BuildViewMtx()
{
	float x = mCameraRadius * cosf(mCameraRotationY);
	float z = mCameraRadius * sinf(mCameraRotationY);
	D3DXVECTOR3 pos(x, mCameraHeight, z);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&mView, &pos, &target, &up);

	HR(mFx->SetValue(mhEyePos, &pos, sizeof(D3DXVECTOR3)));
}

void AmbientDiffuseSpecularDemoApp::BuildProjMtx()
{
	float w = (float)mD3dpp.BackBufferWidth;
	float h = (float)mD3dpp.BackBufferHeight;
	D3DXMatrixPerspectiveFovLH(&mProj, D3DX_PI * 0.25f, w / h, 1.0f, 5000.0f);
}
