#include "DirectInput.h"
#include "Camera.h"
#include "ShadowMapDemoApp.h"
#include <ctime>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	srand(time(0));

	Camera camera;
	gCamera = &camera;

	ShadowMapDemoApp app(hInstance, "Hello Direct3D", D3DDEVTYPE_HAL, D3DCREATE_HARDWARE_VERTEXPROCESSING);
	gD3dApp = &app;

	DirectInput di(DISCL_NONEXCLUSIVE | DISCL_FOREGROUND, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	gDInput = &di;

	if (!gD3dApp->CheckDeviceCaps())
		return 0;
	else
		return gD3dApp->Run();
}