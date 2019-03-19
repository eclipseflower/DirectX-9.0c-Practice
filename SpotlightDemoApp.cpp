#include "SpotlightDemoApp.h"
#include "Vertex.h"

SpotlightDemoApp::SpotlightDemoApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP) : D3DApp(hInstance, winCaption, devType, requestedVP)
{
	if (!CheckDeviceCaps())
	{
		MessageBox(0, "checkDeviceCaps() Failed", 0, 0);
		PostQuitMessage(0);
	}

	mGfxStats = new GfxStats();

	mCameraRadius = 50.0f;
	mCameraRotationY = 1.2 * D3DX_PI;
	mCameraHeight = 20.0f;

	mAmbientLight = 0.4f * _WHITE;
	mDiffuseLight = _WHITE;
	mSpecLight = _WHITE;
	mAttenuation012 = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
	mSpotPower = 16.0f;

	mGridMtrl = Mtrl(_BLUE, _BLUE, _WHITE, 16.0f);
	mCylinderMtrl = Mtrl(_RED, _RED, _WHITE, 8.0f);
	mSphereMtrl = Mtrl(_GREEN, _GREEN, _WHITE, 8.0f);

	HR(D3DXCreateCylinder(gD3dDevice, 1.0f, 1.0f, 6.0f, 20, 20, &mCylinder, 0));
	HR(D3DXCreateSphere(gD3dDevice, 1.0f, 20, 20, &mSphere, 0));

	BuildGeoBuffers();
	BuildFx();

	int numCylVerts = mCylinder->GetNumVertices() * 14;
	int numSphereVerts = mSphere->GetNumVertices() * 14;
	int numCylTris = mCylinder->GetNumFaces() * 14;
	int numSphereTris = mSphere->GetNumFaces() * 14;

	mGfxStats->AddVertices(mNumGridVertices);
	mGfxStats->AddVertices(numCylVerts);
	mGfxStats->AddVertices(numSphereVerts);
	mGfxStats->AddTriangles(mNumGridTriangles);
	mGfxStats->AddTriangles(numCylTris);
	mGfxStats->AddTriangles(numSphereTris);

	OnResetDevice();

	InitAllVertexDeclarations();
}

SpotlightDemoApp::~SpotlightDemoApp()
{
}

bool SpotlightDemoApp::CheckDeviceCaps()
{
	return false;
}

void SpotlightDemoApp::OnLostDevice()
{
}

void SpotlightDemoApp::OnResetDevice()
{
}

void SpotlightDemoApp::UpdateScene(float dt)
{
}

void SpotlightDemoApp::DrawScene()
{
}

void SpotlightDemoApp::BuildGeoBuffers()
{
}

void SpotlightDemoApp::BuildFx()
{
}

void SpotlightDemoApp::BuildViewMtx()
{
}

void SpotlightDemoApp::BuildProjMtx()
{
}

void SpotlightDemoApp::DrawGrid()
{
}

void SpotlightDemoApp::DrawCylinders()
{
}

void SpotlightDemoApp::DrawSpheres()
{
}
