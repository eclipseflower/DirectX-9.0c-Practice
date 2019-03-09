#include "D3DApp.h"

D3DApp* gD3dApp = NULL;
IDirect3DDevice9* gD3dDevice = NULL;

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	if (gD3dApp != NULL)
		return gD3dApp->MsgProc(msg, wParam, lParam);
	else
		return DefWindowProc(hwnd, msg, wParam, lParam);
}

D3DApp::D3DApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP)
{
	mMainWndCaption = winCaption;
	mDevType = devType;
	mRequestedVP = requestedVP;

	mAppInst = hInstance;
	mMainWnd = NULL;
	mD3dObject = NULL;
	mAppPaused = false;
	ZeroMemory(&mD3dpp, sizeof(mD3dpp));

	InitMainWindow();
	InitDirect3D();
}

D3DApp::~D3DApp()
{
	ReleaseCOM(mD3dObject);
	ReleaseCOM(gD3dDevice);
}

HINSTANCE D3DApp::GetAppInst()
{
	return mAppInst;
}

HWND D3DApp::GetMainWnd()
{
	return mMainWnd;
}

void D3DApp::InitMainWindow()
{
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = mAppInst;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "D3DWndClassName";

	if (!RegisterClass(&wc))
	{
		MessageBox(0, "RegisterClass FAILED", 0, 0);
		PostQuitMessage(0);
	}

	RECT r = { 0, 0, 800, 600 };
	AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, false);
	mMainWnd = CreateWindow("D3DWndClassName", mMainWndCaption.c_str(), WS_OVERLAPPEDWINDOW, 100, 100, r.right, r.bottom, NULL, NULL, mAppInst, 0);

	if (!mMainWnd)
	{
		MessageBox(0, "CreateWindow FAILED", 0, 0);
		PostQuitMessage(0);
	}

	ShowWindow(mMainWnd, SW_SHOW);
	UpdateWindow(mMainWnd);
}

void D3DApp::InitDirect3D()
{
	mD3dObject = Direct3DCreate9(D3D_SDK_VERSION);
	if (!mD3dObject)
	{
		MessageBox(0, "Direct3DCreate9 FAILED", 0, 0);
		PostQuitMessage(0);
	}

	D3DDISPLAYMODE mode;
	mD3dObject->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &mode);
	HR(mD3dObject->CheckDeviceType(D3DADAPTER_DEFAULT, mDevType, mode.Format, mode.Format, true));
	HR(mD3dObject->CheckDeviceType(D3DADAPTER_DEFAULT, mDevType, D3DFMT_X8R8G8B8, D3DFMT_X8R8G8B8, false));

	D3DCAPS9 caps;
	HR(mD3dObject->GetDeviceCaps(D3DADAPTER_DEFAULT, mDevType, &caps));

	DWORD devBehaviorFlags = 0;
	if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
		devBehaviorFlags |= mRequestedVP;
	else
		devBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	if (caps.DevCaps & D3DDEVCAPS_PUREDEVICE && devBehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING)
		devBehaviorFlags |= D3DCREATE_PUREDEVICE;

	mD3dpp.BackBufferWidth = 0;
	mD3dpp.BackBufferHeight = 0;
	mD3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	mD3dpp.BackBufferCount = 1;
	mD3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	mD3dpp.MultiSampleQuality = 0;
	mD3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	mD3dpp.hDeviceWindow = mMainWnd;
	mD3dpp.Windowed = true;
	mD3dpp.EnableAutoDepthStencil = true;
	mD3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	mD3dpp.Flags = 0;
	mD3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	mD3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	HR(mD3dObject->CreateDevice(D3DADAPTER_DEFAULT, mDevType, mMainWnd, devBehaviorFlags, &mD3dpp, &gD3dDevice));
}

int D3DApp::Run()
{
	MSG msg;
	msg.message = WM_NULL;

	__int64 cntsPerSec = 0;
	QueryPerformanceCounter((LARGE_INTEGER *)&cntsPerSec);
	float secsPerCnt = 1.0f / (float)cntsPerSec;

	__int64 prevTimeStamp = 0;
	QueryPerformanceFrequency((LARGE_INTEGER *)&prevTimeStamp);

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			if (mAppPaused)
			{
				Sleep(20);
				continue;
			}

			if (!IsDeviceLost())
			{
				__int64 currTimeStamp = 0;
				QueryPerformanceCounter((LARGE_INTEGER *)&currTimeStamp);
				float dt = (currTimeStamp - prevTimeStamp) * secsPerCnt;
				UpdateScene(dt);
				DrawScene();
				prevTimeStamp = currTimeStamp;
			}
		}
	}
	return (int)msg.wParam;
}

LRESULT D3DApp::MsgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool minOrMaxed = false;
	RECT clientRect = { 0, 0, 0, 0 };

	switch (msg)
	{
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
			mAppPaused = true;
		else
			mAppPaused = false;
		break;

	case WM_SIZE:
		if (gD3dDevice)
		{
			mD3dpp.BackBufferWidth = LOWORD(lParam);
			mD3dpp.BackBufferHeight = HIWORD(lParam);
			if (wParam == SIZE_MINIMIZED)
			{
				mAppPaused = true;
				minOrMaxed = true;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				mAppPaused = false;
				minOrMaxed = true;
				OnLostDevice();
				HR(gD3dDevice->Reset(&mD3dpp));
				OnResetDevice();
			}
			else if (wParam == SIZE_RESTORED)
			{
				mAppPaused = false;
				if (minOrMaxed && mD3dpp.Windowed)
				{
					OnLostDevice();
					HR(gD3dDevice->Reset(&mD3dpp));
					OnResetDevice();
				}
				minOrMaxed = false;
			}
		}
		break;

	case WM_EXITSIZEMOVE:
		GetClientRect(mMainWnd, &clientRect);
		mD3dpp.BackBufferWidth = clientRect.right;
		mD3dpp.BackBufferHeight = clientRect.bottom;
		OnLostDevice();
		HR(gD3dDevice->Reset(&mD3dpp));
		OnResetDevice();
		break;

	case WM_CLOSE:
		DestroyWindow(mMainWnd);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
			EnableFullScreenMode(false);
		else if (wParam == 'F')
			EnableFullScreenMode(true);
		break;

	default:
		break;
	}
	return DefWindowProc(mMainWnd, msg, wParam, lParam);
}

void D3DApp::EnableFullScreenMode(bool enable)
{
	if (enable)
	{
		if (!mD3dpp.Windowed)
			return;

		int width = GetSystemMetrics(SM_CXSCREEN);
		int height = GetSystemMetrics(SM_CYSCREEN);

		mD3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
		mD3dpp.BackBufferWidth = width;
		mD3dpp.BackBufferHeight = height;
		mD3dpp.Windowed = false;

		SetWindowLongPtr(mMainWnd, GWL_STYLE, WS_POPUP);
		SetWindowPos(mMainWnd, HWND_TOP, 0, 0, width, height, SWP_NOZORDER | SWP_SHOWWINDOW);
	}
	else
	{
		if (mD3dpp.Windowed)
			return;

		RECT r = { 0, 0, 800, 600 };
		AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, false);
		mD3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
		mD3dpp.BackBufferWidth = 800;
		mD3dpp.BackBufferHeight = 600;
		mD3dpp.Windowed = true;

		SetWindowLongPtr(mMainWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
		SetWindowPos(mMainWnd, HWND_TOP, 100, 100, r.right, r.bottom, SWP_NOZORDER | SWP_SHOWWINDOW);
	}

	OnLostDevice();
	HR(gD3dDevice->Reset(&mD3dpp));
	OnResetDevice();
}


bool D3DApp::IsDeviceLost()
{
	HRESULT hr = gD3dDevice->TestCooperativeLevel();

	if (hr == D3DERR_DEVICELOST)
	{
		Sleep(20);
		return true;
	}
	else if (hr == D3DERR_DRIVERINTERNALERROR)
	{
		MessageBox(0, "Internal Driver Error...Exiting", 0, 0);
		PostQuitMessage(0);
		return true;
	}
	else if (hr == D3DERR_DEVICENOTRESET)
	{
		OnLostDevice();
		HR(gD3dDevice->Reset(&mD3dpp));
		OnResetDevice();
		return false;
	}
	else
		return false;
}
