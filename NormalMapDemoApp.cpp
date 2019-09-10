#include "NormalMapDemoApp.h"
#include "DirectInput.h"
#include "Camera.h"
#include "Vertex.h"

NormalMapDemoApp::NormalMapDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP) : D3DApp(hInstance, winCaption, devType, requestedVP)
{
	if (!CheckDeviceCaps())
	{
		MessageBox(0, "checkDeviceCaps() Failed", 0, 0);
		PostQuitMessage(0);
	}

	InitAllVertexDeclarations();

	mGfxStats = new GfxStats();
	mSky = new Sky("grassenvmap1024.dds", 10000.0f);

	mLight.ambient = D3DXCOLOR(0.4f, 0.4f, 0.4f, 1.0f);
	mLight.diffuse = D3DXCOLOR(0.8f, 0.8f, 0.8f, 1.0f);
	mLight.spec = D3DXCOLOR(0.6f, 0.6f, 0.6f, 1.0f);

	ID3DXMesh* tempMesh = 0;
	LoadXFile("BasicColumnScene.x", &tempMesh, mSceneMtrls, mSceneTextures);

	// Get the vertex declaration for the NMapVertex.
	D3DVERTEXELEMENT9 elems[MAX_FVF_DECL_SIZE];
	UINT numElems = 0;
	HR(NMapVertex::decl->GetDeclaration(elems, &numElems));

	// Clone the mesh to the NMapVertex format.
	ID3DXMesh* clonedTempMesh = 0;
	HR(tempMesh->CloneMesh(D3DXMESH_MANAGED, elems, gD3dDevice, &clonedTempMesh));

	// Now use D3DXComputeTangentFrameEx to build the TNB-basis for each vertex
	// in the mesh.  

	HR(D3DXComputeTangentFrameEx(
		clonedTempMesh, // Input mesh
		D3DDECLUSAGE_TEXCOORD, 0, // Vertex element of input tex-coords.  
		D3DDECLUSAGE_BINORMAL, 0, // Vertex element to output binormal.
		D3DDECLUSAGE_TANGENT, 0,  // Vertex element to output tangent.
		D3DDECLUSAGE_NORMAL, 0,   // Vertex element to output normal.
		0, // Options
		0, // Adjacency
		0.01f, 0.25f, 0.01f, // Thresholds for handling errors
		&mSceneMesh, // Output mesh
		0));         // Vertex Remapping

	  // Done with temps.
	ReleaseCOM(tempMesh);
	ReleaseCOM(clonedTempMesh);

	D3DXMatrixIdentity(&mSceneWorld);
	D3DXMatrixIdentity(&mSceneWorldInv);

	HR(D3DXCreateTextureFromFile(gD3dDevice, "floor_nmap.bmp", &mSceneNormalMaps[0]));
	HR(D3DXCreateTextureFromFile(gD3dDevice, "bricks_nmap.bmp", &mSceneNormalMaps[1]));

	HR(D3DXCreateTextureFromFile(gD3dDevice, "whitetex.dds", &mWhiteTex));

	// Initialize camera.
	gCamera->Pos().y = 3.0f;
	gCamera->Pos().z = -10.0f;
	gCamera->SetSpeed(10.0f);

	mGfxStats->AddVertices(mSceneMesh->GetNumVertices());
	mGfxStats->AddTriangles(mSceneMesh->GetNumFaces());

	mGfxStats->AddVertices(mSky->GetNumVertices());
	mGfxStats->AddTriangles(mSky->GetNumTriangles());

	BuildFX();

	OnResetDevice();
}

NormalMapDemoApp::~NormalMapDemoApp()
{
	delete mGfxStats;
	delete mSky;

	ReleaseCOM(mFX);

	ReleaseCOM(mSceneMesh);
	for (UINT i = 0; i < mSceneTextures.size(); ++i)
		ReleaseCOM(mSceneTextures[i]);

	ReleaseCOM(mWhiteTex);
	ReleaseCOM(mSceneNormalMaps[0]);
	ReleaseCOM(mSceneNormalMaps[1]);

	DestroyAllVertexDeclarations();
}

bool NormalMapDemoApp::CheckDeviceCaps()
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

void NormalMapDemoApp::OnLostDevice()
{
	mGfxStats->OnLostDevice();
	mSky->OnLostDevice();
	HR(mFX->OnLostDevice());
}

void NormalMapDemoApp::OnResetDevice()
{
	mGfxStats->OnResetDevice();
	mSky->OnResetDevice();
	HR(mFX->OnResetDevice());

	// The aspect ratio depends on the backbuffer dimensions, which can 
	// possibly change after a reset.  So rebuild the projection matrix.
	float w = (float)mD3dpp.BackBufferWidth;
	float h = (float)mD3dpp.BackBufferHeight;
	gCamera->SetLens(D3DX_PI * 0.25f, w / h, 1.0f, 5000.0f);
}

void NormalMapDemoApp::UpdateScene(float dt)
{
	mGfxStats->Update(dt);

	gDInput->poll();

	gCamera->Update(dt, 0, 0);


	// Animate light by spinning it around.

	static float time = 0.0f;
	time += dt;

	mLight.dirW.x = 5.0f*cosf(time);
	mLight.dirW.y = -1.0f;
	mLight.dirW.z = 5.0f*sinf(time);

	D3DXVec3Normalize(&mLight.dirW, &mLight.dirW);
}

void NormalMapDemoApp::DrawScene()
{
	HR(gD3dDevice->BeginScene());

	mSky->Draw();

	HR(mFX->SetValue(mhLight, &mLight, sizeof(DirLight)));
	HR(mFX->SetMatrix(mhWVP, &(mSceneWorld*gCamera->ViewProj())));
	HR(mFX->SetValue(mhEyePosW, &gCamera->Pos(), sizeof(D3DXVECTOR3)));

	UINT numPasses = 0;
	HR(mFX->Begin(&numPasses, 0));
	HR(mFX->BeginPass(0));

	for (UINT j = 0; j < mSceneMtrls.size(); ++j)
	{
		HR(mFX->SetValue(mhMtrl, &mSceneMtrls[j], sizeof(Mtrl)));

		// If there is a texture, then use.
		if (mSceneTextures[j] != 0)
		{
			HR(mFX->SetTexture(mhTex, mSceneTextures[j]));
		}

		// But if not, then set a pure white texture.  When the texture color
		// is multiplied by the color from lighting, it is like multiplying by
		// 1 and won't change the color from lighting.
		else
		{
			HR(mFX->SetTexture(mhTex, mWhiteTex));
		}

		HR(mFX->SetTexture(mhNormalMap, mSceneNormalMaps[j]));

		HR(mFX->CommitChanges());
		HR(mSceneMesh->DrawSubset(j));
	}
	HR(mFX->EndPass());
	HR(mFX->End());

	mGfxStats->Display();

	HR(gD3dDevice->EndScene());

	// Present the backbuffer.
	HR(gD3dDevice->Present(0, 0, 0, 0));
}

void NormalMapDemoApp::BuildFX()
{
	ID3DXBuffer* errors = 0;
	HR(D3DXCreateEffectFromFile(gD3dDevice, "NormalMap.fx",
		0, 0, D3DXSHADER_DEBUG, 0, &mFX, &errors));
	if (errors)
		MessageBox(0, (char*)errors->GetBufferPointer(), 0, 0);

	// Obtain handles.
	mhTech = mFX->GetTechniqueByName("NormalMapTech");
	mhWVP = mFX->GetParameterByName(0, "gWVP");
	mhWorldInv = mFX->GetParameterByName(0, "gWorldInv");
	mhMtrl = mFX->GetParameterByName(0, "gMtrl");
	mhLight = mFX->GetParameterByName(0, "gLight");
	mhEyePosW = mFX->GetParameterByName(0, "gEyePosW");
	mhTex = mFX->GetParameterByName(0, "gTex");
	mhNormalMap = mFX->GetParameterByName(0, "gNormalMap");

	// Set parameters that do not vary:

	// World is the identity, so inverse is also identity.
	HR(mFX->SetMatrix(mhWorldInv, &mSceneWorldInv));
	HR(mFX->SetTechnique(mhTech));
}
