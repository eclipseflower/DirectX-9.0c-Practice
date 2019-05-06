#include "Vertex.h"
#include "DirectInput.h"
#include "RobotArmDemoApp.h"


RobotArmDemoApp::RobotArmDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP) : D3DApp(hInstance, winCaption, devType, requestedVP)
{
	if (!CheckDeviceCaps())
	{
		MessageBox(0, "checkDeviceCaps() Failed", 0, 0);
		PostQuitMessage(0);
	}

	InitAllVertexDeclarations();

	mGfxStats = new GfxStats();

	// Initialize Camera Settings
	mCameraRadius = 9.0f;
	mCameraRotationY = 1.5f * D3DX_PI;
	mCameraHeight = 0.0f;

	// Setup a directional light.
	mLight.dirW = D3DXVECTOR3(0.0f, 1.0f, 2.0f);
	D3DXVec3Normalize(&mLight.dirW, &mLight.dirW);
	mLight.ambient = D3DXCOLOR(0.8f, 0.8f, 0.8f, 1.0f);
	mLight.diffuse = D3DXCOLOR(0.8f, 0.8f, 0.8f, 1.0f);
	mLight.spec = D3DXCOLOR(0.8f, 0.8f, 0.8f, 1.0f);

	// Load the bone .X file mesh.
	LoadXFile("bone.x", &mBoneMesh, mMtrl, mTex);
	D3DXMatrixIdentity(&mWorld);

	// Create the white dummy texture.
	HR(D3DXCreateTextureFromFile(gD3dDevice, "whitetex.dds", &mWhiteTex));

	// Initialize the bones relative to their parent frame.
	// The root is special--its parent frame is the world space.
	// As such, its position and angle are ignored--its 
	// toWorldXForm should be set explicitly (that is, the world
	// transform of the entire skeleton).
	//
	// *------*------*------*------
	//    0      1      2      3

	for (int i = 1; i < NUM_BONES; ++i) // Ignore root.
	{
		// Describe each bone frame relative to its parent frame.
		mBones[i].pos = D3DXVECTOR3(2.0f, 0.0f, 0.0f);
		mBones[i].zAngle = 0.0f;
	}
	// Root frame at center of world.
	mBones[0].pos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	mBones[0].zAngle = 0.0f;


	// Start off with the last (leaf) bone:
	mBoneSelected = NUM_BONES - 1;

	mGfxStats->AddVertices(mBoneMesh->GetNumVertices() * NUM_BONES);
	mGfxStats->AddTriangles(mBoneMesh->GetNumFaces() * NUM_BONES);

	BuildFX();

	OnResetDevice();
}

RobotArmDemoApp::~RobotArmDemoApp()
{
	delete mGfxStats;

	ReleaseCOM(mFX);

	ReleaseCOM(mBoneMesh);
	for (int i = 0; i < mTex.size(); ++i)
		ReleaseCOM(mTex[i]);

	ReleaseCOM(mWhiteTex);

	DestroyAllVertexDeclarations();
}

bool RobotArmDemoApp::CheckDeviceCaps()
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

void RobotArmDemoApp::OnLostDevice()
{
	mGfxStats->OnLostDevice();
	HR(mFX->OnLostDevice());
}

void RobotArmDemoApp::OnResetDevice()
{
	mGfxStats->OnResetDevice();
	HR(mFX->OnResetDevice());


	// The aspect ratio depends on the backbuffer dimensions, which can 
	// possibly change after a reset.  So rebuild the projection matrix.
	BuildProjMtx();
}

void RobotArmDemoApp::UpdateScene(float dt)
{
	mGfxStats->Update(dt);

	// Get snapshot of input devices.
	gDInput->poll();

	// Check input.
	if (gDInput->keyDown(DIK_W))
		mCameraHeight += 25.0f * dt;
	if (gDInput->keyDown(DIK_S))
		mCameraHeight -= 25.0f * dt;

	// Allow the user to select a bone (zero based index)
	if (gDInput->keyDown(DIK_1))	mBoneSelected = 0;
	if (gDInput->keyDown(DIK_2))	mBoneSelected = 1;
	if (gDInput->keyDown(DIK_3))	mBoneSelected = 2;
	if (gDInput->keyDown(DIK_4))	mBoneSelected = 3;
	if (gDInput->keyDown(DIK_5))	mBoneSelected = 4;

	// Allow the user to rotate a bone.
	if (gDInput->keyDown(DIK_A))
		mBones[mBoneSelected].zAngle += 1.0f * dt;
	if (gDInput->keyDown(DIK_D))
		mBones[mBoneSelected].zAngle -= 1.0f * dt;

	// If we rotate over 360 degrees, just roll back to 0
	if (fabsf(mBones[mBoneSelected].zAngle) >= 2.0f*D3DX_PI)
		mBones[mBoneSelected].zAngle = 0.0f;


	// Divide by 50 to make mouse less sensitive. 
	mCameraRotationY += gDInput->mouseDX() / 100.0f;
	mCameraRadius += gDInput->mouseDY() / 25.0f;

	// If we rotate over 360 degrees, just roll back to 0
	if (fabsf(mCameraRotationY) >= 2.0f * D3DX_PI)
		mCameraRotationY = 0.0f;

	// Don't let radius get too small.
	if (mCameraRadius < 2.0f)
		mCameraRadius = 2.0f;

	// The camera position/orientation relative to world space can 
	// change every frame based on input, so we need to rebuild the
	// view matrix every frame with the latest changes.
	BuildViewMtx();
}

void RobotArmDemoApp::DrawScene()
{
	HR(gD3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0));

	HR(gD3dDevice->BeginScene());

	HR(mFX->SetValue(mhLight, &mLight, sizeof(DirLight)));

	HR(mFX->SetTechnique(mhTech));
	UINT numPasses = 0;
	HR(mFX->Begin(&numPasses, 0));
	HR(mFX->BeginPass(0));

	// Build the world transforms for each bone, then render them.
	BuildBoneWorldTransforms();
	D3DXMATRIX T;
	D3DXMatrixTranslation(&T, -NUM_BONES, 0.0f, 0.0f);
	for (int i = 0; i < NUM_BONES; ++i)
	{
		// Append the transformation with a slight translation to better
		// center the skeleton at the center of the scene.
		mWorld = mBones[i].toWorldXForm * T;
		HR(mFX->SetMatrix(mhWVP, &(mWorld*mView*mProj)));
		D3DXMATRIX worldInvTrans;
		D3DXMatrixInverse(&worldInvTrans, 0, &mWorld);
		D3DXMatrixTranspose(&worldInvTrans, &worldInvTrans);
		HR(mFX->SetMatrix(mhWorldInvTrans, &worldInvTrans));
		HR(mFX->SetMatrix(mhWorld, &mWorld));
		for (int j = 0; j < mMtrl.size(); ++j)
		{
			HR(mFX->SetValue(mhMtrl, &mMtrl[j], sizeof(Mtrl)));

			// If there is a texture, then use.
			if (mTex[j] != 0)
			{
				HR(mFX->SetTexture(mhTex, mTex[j]));
			}

			// But if not, then set a pure white texture.  When the texture color
			// is multiplied by the color from lighting, it is like multiplying by
			// 1 and won't change the color from lighting.
			else
			{
				HR(mFX->SetTexture(mhTex, mWhiteTex));
			}

			HR(mFX->CommitChanges());
			HR(mBoneMesh->DrawSubset(j));
		}
	}

	HR(mFX->EndPass());
	HR(mFX->End());

	mGfxStats->Display();

	HR(gD3dDevice->EndScene());

	// Present the backbuffer.
	HR(gD3dDevice->Present(0, 0, 0, 0));
}

void RobotArmDemoApp::BuildFX()
{
	ID3DXBuffer* errors = 0;
	HR(D3DXCreateEffectFromFile(gD3dDevice, "phongdirlttex.fx", 0, 0, D3DXSHADER_DEBUG, 0, &mFX, &errors));
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
}

void RobotArmDemoApp::BuildViewMtx()
{
	float x = mCameraRadius * cosf(mCameraRotationY);
	float z = mCameraRadius * sinf(mCameraRotationY);
	D3DXVECTOR3 pos(x, mCameraHeight, z);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&mView, &pos, &target, &up);

	HR(mFX->SetValue(mhEyePos, &pos, sizeof(D3DXVECTOR3)));
}

void RobotArmDemoApp::BuildProjMtx()
{
	float w = (float)mD3dpp.BackBufferWidth;
	float h = (float)mD3dpp.BackBufferHeight;
	D3DXMatrixPerspectiveFovLH(&mProj, D3DX_PI * 0.25f, w / h, 1.0f, 5000.0f);
}

void RobotArmDemoApp::BuildBoneWorldTransforms()
{
	D3DXMATRIX R, T;
	D3DXVECTOR3 p;
	for (int i = 0; i < NUM_BONES; ++i)
	{
		p = mBones[i].pos;
		D3DXMatrixRotationZ(&R, mBones[i].zAngle);
		D3DXMatrixTranslation(&T, p.x, p.y, p.z);
		mBones[i].toParentXForm = R * T;
	}


	// Now, the ith object's world transform is given by its 
	// to-parent transform, followed by its parent's to-parent transform, 
	// followed by its grandparent's to-parent transform, and
	// so on, up to the root's to-parent transform.

	// For each bone...
	for (int i = 0; i < NUM_BONES; ++i)
	{
		// Initialize to identity matrix.
		D3DXMatrixIdentity(&mBones[i].toWorldXForm);

		// Combine  W[i] = W[i]*W[i-1]*...*W[0].
		for (int j = i; j >= 0; --j)
		{
			mBones[i].toWorldXForm *= mBones[j].toParentXForm;
		}
	}
}
