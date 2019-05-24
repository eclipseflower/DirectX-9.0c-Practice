#include "BasicTerrainDemoApp.h"
#include "DirectInput.h"
#include "Vertex.h"

BasicTerrainDemoApp::BasicTerrainDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP) : D3DApp(hInstance, winCaption, devType, requestedVP)
{
	if (!CheckDeviceCaps())
	{
		MessageBox(0, "checkDeviceCaps() Failed", 0, 0);
		PostQuitMessage(0);
	}

	InitAllVertexDeclarations();

	mGfxStats = new GfxStats();

	mCameraRadius = 80.0f;
	mCameraRotationY = 1.2 * D3DX_PI;
	mCameraHeight = 40.0f;

	D3DXMatrixIdentity(&mWorld);

	mHeightmap.LoadRAW(129, 129, "heightmap17_129.raw", 0.25f, 0.0f);

	// Load textures from file.
	HR(D3DXCreateTextureFromFile(gD3dDevice, "grass.dds", &mTex0));
	HR(D3DXCreateTextureFromFile(gD3dDevice, "dirt.dds", &mTex1));
	HR(D3DXCreateTextureFromFile(gD3dDevice, "stone.dds", &mTex2));
	HR(D3DXCreateTextureFromFile(gD3dDevice, "blend_hm17.dds", &mBlendMap));

	BuildGridGeometry();
	mGfxStats->AddVertices(mTerrainMesh->GetNumVertices());
	mGfxStats->AddTriangles(mTerrainMesh->GetNumFaces());

	BuildFX();

	OnResetDevice();
}

BasicTerrainDemoApp::~BasicTerrainDemoApp()
{
	delete mGfxStats;
	ReleaseCOM(mTerrainMesh);
	ReleaseCOM(mTex0);
	ReleaseCOM(mTex1);
	ReleaseCOM(mTex2);
	ReleaseCOM(mBlendMap);
	ReleaseCOM(mFX);

	DestroyAllVertexDeclarations();
}

bool BasicTerrainDemoApp::CheckDeviceCaps()
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

void BasicTerrainDemoApp::OnLostDevice()
{
	mGfxStats->OnLostDevice();
	HR(mFX->OnLostDevice());
}

void BasicTerrainDemoApp::OnResetDevice()
{
	mGfxStats->OnResetDevice();
	HR(mFX->OnResetDevice());

	BuildProjMtx();
}

void BasicTerrainDemoApp::UpdateScene(float dt)
{
	mGfxStats->Update(dt);

	// Get snapshot of input devices.
	gDInput->poll();

	// Check input.
	if (gDInput->keyDown(DIK_W))
		mCameraHeight += 25.0f * dt;
	if (gDInput->keyDown(DIK_S))
		mCameraHeight -= 25.0f * dt;

	// Divide by 50 to make mouse less sensitive. 
	mCameraRotationY += gDInput->mouseDX() / 100.0f;
	mCameraRadius += gDInput->mouseDY() / 25.0f;

	// If we rotate over 360 degrees, just roll back to 0
	if (fabsf(mCameraRotationY) >= 2.0f * D3DX_PI)
		mCameraRotationY = 0.0f;

	// Don't let radius get too small.
	if (mCameraRadius < 5.0f)
		mCameraRadius = 5.0f;

	// The camera position/orientation relative to world space can 
	// change every frame based on input, so we need to rebuild the
	// view matrix every frame with the latest changes.
	BuildViewMtx();
}

void BasicTerrainDemoApp::DrawScene()
{
	HR(gD3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffeeeeee, 1.0f, 0));

	HR(gD3dDevice->BeginScene());

	// Setup the rendering FX
	HR(mFX->SetMatrix(mhViewProj, &(mView*mProj)));
	HR(mFX->SetTechnique(mhTech));
	UINT numPasses = 0;
	HR(mFX->Begin(&numPasses, 0));
	HR(mFX->BeginPass(0));

	HR(mTerrainMesh->DrawSubset(0));

	HR(mFX->EndPass());
	HR(mFX->End());

	mGfxStats->Display();

	HR(gD3dDevice->EndScene());

	// Present the backbuffer.
	HR(gD3dDevice->Present(0, 0, 0, 0));
}

void BasicTerrainDemoApp::BuildGridGeometry()
{
	std::vector<D3DXVECTOR3> verts;
	std::vector<DWORD> indices;

	int vertRows = 129;
	int vertCols = 129;
	float dx = 1.0f;
	float dz = 1.0f;

	GenTriGrid(vertRows, vertCols, dx, dz,
		D3DXVECTOR3(0.0f, 0.0f, 0.0f), verts, indices);

	int numVerts = vertRows * vertCols;
	int numTris = (vertRows - 1)*(vertCols - 1) * 2;

	// Create the mesh.
	D3DVERTEXELEMENT9 elems[MAX_FVF_DECL_SIZE];
	UINT numElems = 0;
	HR(VertexPNT::decl->GetDeclaration(elems, &numElems));
	HR(D3DXCreateMesh(numTris, numVerts,
		D3DXMESH_MANAGED, elems, gD3dDevice, &mTerrainMesh));

	// Write the vertices.
	VertexPNT* v = 0;
	HR(mTerrainMesh->LockVertexBuffer(0, (void**)&v));

	// width/depth
	float w = (vertCols - 1) * dx;
	float d = (vertRows - 1) * dz;
	for (int i = 0; i < vertRows; ++i)
	{
		for (int j = 0; j < vertCols; ++j)
		{
			DWORD index = i * vertCols + j;
			v[index].pos = verts[index];
			v[index].pos.y = mHeightmap(i, j);
			v[index].normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
			v[index].tex0.x = (v[index].pos.x + (0.5f*w)) / w;
			v[index].tex0.y = (v[index].pos.z - (0.5f*d)) / -d;
		}
	}

	HR(mTerrainMesh->UnlockVertexBuffer());


	// Write the indices and attribute buffer.
	WORD* k = 0;
	HR(mTerrainMesh->LockIndexBuffer(0, (void**)&k));
	DWORD* attBuffer = 0;
	HR(mTerrainMesh->LockAttributeBuffer(0, &attBuffer));

	// Compute the indices for each triangle.
	for (int i = 0; i < numTris; ++i)
	{
		k[i * 3 + 0] = (WORD)indices[i * 3 + 0];
		k[i * 3 + 1] = (WORD)indices[i * 3 + 1];
		k[i * 3 + 2] = (WORD)indices[i * 3 + 2];

		attBuffer[i] = 0; // Always subset 0
	}

	HR(mTerrainMesh->UnlockIndexBuffer());
	HR(mTerrainMesh->UnlockAttributeBuffer());

	// Generate normals and then optimize the mesh.
	HR(D3DXComputeNormals(mTerrainMesh, 0));

	DWORD* adj = new DWORD[mTerrainMesh->GetNumFaces() * 3];
	HR(mTerrainMesh->GenerateAdjacency(EPSILON, adj));
	HR(mTerrainMesh->OptimizeInplace(D3DXMESHOPT_VERTEXCACHE | D3DXMESHOPT_ATTRSORT,
		adj, 0, 0, 0));
	delete[] adj;
}

void BasicTerrainDemoApp::BuildFX()
{
	ID3DXBuffer* errors = 0;
	HR(D3DXCreateEffectFromFile(gD3dDevice, "terrain.fx",
		0, 0, D3DXSHADER_DEBUG, 0, &mFX, &errors));
	if (errors)
		MessageBox(0, (char*)errors->GetBufferPointer(), 0, 0);

	mhTech = mFX->GetTechniqueByName("TerrainTech");
	mhViewProj = mFX->GetParameterByName(0, "gViewProj");
	mhDirToSunW = mFX->GetParameterByName(0, "gDirToSunW");
	mhTex0 = mFX->GetParameterByName(0, "gTex0");
	mhTex1 = mFX->GetParameterByName(0, "gTex1");
	mhTex2 = mFX->GetParameterByName(0, "gTex2");
	mhBlendMap = mFX->GetParameterByName(0, "gBlendMap");

	HR(mFX->SetTexture(mhTex0, mTex0));
	HR(mFX->SetTexture(mhTex1, mTex1));
	HR(mFX->SetTexture(mhTex2, mTex2));
	HR(mFX->SetTexture(mhBlendMap, mBlendMap));

	D3DXVECTOR3 d(0.0f, 1.0f, 0.0f);
	HR(mFX->SetValue(mhDirToSunW, &d, sizeof(D3DXVECTOR3)));
}

void BasicTerrainDemoApp::BuildViewMtx()
{
	float x = mCameraRadius * cosf(mCameraRotationY);
	float z = mCameraRadius * sinf(mCameraRotationY);
	D3DXVECTOR3 pos(x, mCameraHeight, z);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&mView, &pos, &target, &up);
}

void BasicTerrainDemoApp::BuildProjMtx()
{
	float w = (float)mD3dpp.BackBufferWidth;
	float h = (float)mD3dpp.BackBufferHeight;
	D3DXMatrixPerspectiveFovLH(&mProj, D3DX_PI * 0.25f, w / h, 1.0f, 5000.0f);
}
