//	DX11Renderer - VDemo | DirectX11 Renderer
//	Copyright(C) 2016  - Volkan Ilbeyli
//
//	This program is free software : you can redistribute it and / or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program.If not, see <http://www.gnu.org/licenses/>.
//
//	Contact: volkanilbeyli@gmail.com

#include "BaseSystem.h"
#include "Input.h"

#include "Engine/Engine.h"
#include "Engine/Settings.h"

#include "Utilities/CustomParser.h"
#include "Utilities/Log.h"

#include <strsafe.h>
#include <vector>
#include <new>

#ifdef _DEBUG
#include <cassert>
#endif

BaseSystem::BaseSystem()
{
	m_hInstance		= GetModuleHandle(NULL);	// instance of this application
	m_appName		= "VQEngine Demo";
}


BaseSystem::BaseSystem(const BaseSystem& other){}

BaseSystem::~BaseSystem(){}

bool BaseSystem::Init()
{
	const Settings::Window& windowSettings = Engine::ReadSettingsFromFile().window;
	InitWindow(windowSettings);

	if (!ENGINE->Initialize(m_hwnd))
	{
		Log::Error("cannot initialize engine. Exiting..");
		return false;
	}

	if (!ENGINE->Load())
	{
		Log::Error("Exiting..");
		return false;
	}

	Log::Info("Engine initialization and asset loading successful.\n");
	return true;
}	

void BaseSystem::Run()
{
	ENGINE->mProfiler->BeginProfile();
	ENGINE->mpTimer->Reset();
	ENGINE->mpTimer->Start();
	MSG msg = { };
	
	bool bExitApp = false;
	while (!bExitApp)
	{
		// todo: keep dragging main window
		// game engine architecture
		// http://advances.realtimerendering.com/s2016/index.html
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);		// Translates virtual-key messages into character messages
			DispatchMessage(&msg);		// indirectly causes Windows to invoke WndProc
		}
		//else
		//{
		if (msg.message == WM_QUIT)	bExitApp = true;
		else						bExitApp = ENGINE->UpdateAndRender();
		//}
	}
	ENGINE->mProfiler->EndProfile();
}

void BaseSystem::Exit()
{
	ENGINE->Exit(); 
	ShutdownWindows();
}

LRESULT CALLBACK BaseSystem::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	const Settings::Window& setting = Engine::sEngineSettings.window;
	switch (umsg)
	{
	// application active/inactive
	case WM_ACTIVATE:
		if (LOWORD(wparam) == WA_INACTIVE)
		{
			// paused = true
			// timer stop
		}
		else
		{
			// paused = false
			// timer start
		}
		break;

	// resize bar grab-release
	case WM_ENTERSIZEMOVE:
		// paused = true
		// resizing = true
		// timer.stop()
		break;

	case WM_EXITSIZEMOVE:
		// paused = false
		// resizing= false
		// timer.start()
		// onresize()
		break;

	// prevent window from becoming too small
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lparam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lparam)->ptMinTrackSize.y = 200;
		break;

	// keyboard
	case WM_KEYDOWN:
	{
		ENGINE->mpInput->KeyDown((KeyCode)wparam);
		break;
	}

	case WM_KEYUP:
	{
		ENGINE->mpInput->KeyUp((KeyCode)wparam);
		break;
	}

	// mouse buttons
	case WM_MBUTTONDOWN:
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	{
		ENGINE->mpInput->KeyDown((KeyCode)wparam);
		break;
	}

	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
	case WM_LBUTTONUP:
	{
		ENGINE->mpInput->KeyUp((KeyCode)wparam);
		break;
	}

#ifdef ENABLE_RAW_INPUT
	// raw input for mouse - see: https://msdn.microsoft.com/en-us/library/windows/desktop/ee418864.aspx
	case WM_INPUT:	
	{
		UINT rawInputSize = 48;
		LPBYTE inputBuffer[48];
		ZeroMemory(inputBuffer, rawInputSize);

		GetRawInputData(
			(HRAWINPUT)lparam, 
			RID_INPUT,
			inputBuffer, 
			&rawInputSize, 
			sizeof(RAWINPUTHEADER));

		RAWINPUT* raw = (RAWINPUT*)inputBuffer;

		if (raw->header.dwType == RIM_TYPEMOUSE &&
			raw->data.mouse.usFlags == MOUSE_MOVE_RELATIVE)
		{
			long xPosRelative = raw->data.mouse.lLastX;
			long yPosRelative = raw->data.mouse.lLastY;
			short scroll = raw->data.mouse.usButtonData;
			ENGINE->mpInput->UpdateMousePos(xPosRelative, yPosRelative, scroll);
			SetCursorPos(setting.width/2, setting.height/2);
			
#ifdef LOG
			char szTempOutput[1024];
			StringCchPrintf(szTempOutput, STRSAFE_MAX_CCH, TEXT("%u  Mouse: usFlags=%04x ulButtons=%04x usButtonFlags=%04x usButtonData=%04x ulRawButtons=%04x lLastX=%04x lLastY=%04x ulExtraInformation=%04x\r\n"),
				rawInputSize,
				raw->data.mouse.usFlags,
				raw->data.mouse.ulButtons,
				raw->data.mouse.usButtonFlags,
				raw->data.mouse.usButtonData,
				raw->data.mouse.ulRawButtons,
				raw->data.mouse.lLastX,
				raw->data.mouse.lLastY,
				raw->data.mouse.ulExtraInformation);
			OutputDebugString(szTempOutput);
#endif
		}
		break;
	}

#else
	// client area mouse - not good for first person camera
	case WM_MOUSEMOVE:
	{
		ENGINE->mpInput->UpdateMousePos(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), scroll);
		break;
	}
#endif

	default:
	{
		return DefWindowProc(hwnd, umsg, wparam, lparam);
	}
	}

	return 0;
}

void BaseSystem::UpdateWindowDimensions(int w, int h)
{
	m_windowHeight = h;
	m_windowWidth  = w;
}

void BaseSystem::InitWindow(const Settings::Window& windowSettings)
{
	int width, height;
	int posX, posY;				// window position
	gp_appHandle	= this;		// global handle		

	// default settings for windows class
	WNDCLASSEX wc;
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc		= WndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= m_hInstance;
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm			= wc.hIcon;
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName		= NULL;
	wc.lpszClassName	= m_appName;
	wc.cbSize			= sizeof(WNDCLASSEX);
	RegisterClassEx(&wc);

	// get clients desktop resolution
	width	= GetSystemMetrics(SM_CXSCREEN);
	height	= GetSystemMetrics(SM_CYSCREEN);

	// set screen settings
	if (windowSettings.fullscreen)
	{
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize			= sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth	= (unsigned long)width;
		dmScreenSettings.dmPelsHeight	= (unsigned long)height;
		dmScreenSettings.dmBitsPerPel	= 32;
		dmScreenSettings.dmFields		= DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		posX = posY = 0;
	}
	else
	{
		width  = windowSettings.width;
		height = windowSettings.height;

		posX = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;
	}

	// create window with screen settings
	m_hwnd = CreateWindowEx(
		WS_EX_APPWINDOW,			// Forces a top-level window onto the taskbar when the window is visible.
		m_appName,					// class name
		m_appName,					// Window name
		WS_OVERLAPPEDWINDOW, //WS_POPUP,					// Window style
		posX, posY, width, height,	// Window position and dimensions
		NULL, NULL,					// parent, menu
		m_hInstance, NULL
		);

	if (m_hwnd == nullptr)
	{
		MessageBox(NULL, "CreateWindowEx() failed", "Error", MB_OK);
		ENGINE->Exit();
		PostQuitMessage(0);
		return;
	}

	// focus window
	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);
	ShowCursor(false);
	//SetCursorPos(posX + width / 2, posY + height / 2);

	RECT rcClip;
	GetWindowRect(m_hwnd, &rcClip);
	ClipCursor(&rcClip);

#ifdef ENABLE_RAW_INPUT
	InitRawInputDevices();
#endif
	return;
}

void BaseSystem::ShutdownWindows()
{
	ShowCursor(true);

	if (Engine::sEngineSettings.window.fullscreen)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	// Remove the window.
	DestroyWindow(m_hwnd);
	m_hwnd = NULL;

	// Remove the application instance.
	UnregisterClass(m_appName, m_hInstance);
	m_hInstance = NULL;

	// Release the pointer to this class.
	gp_appHandle = NULL;

	return;
}

void BaseSystem::InitRawInputDevices()
{
	// register mouse for raw input
	// https://msdn.microsoft.com/en-us/library/windows/desktop/ms645565.aspx
	RAWINPUTDEVICE Rid[1];
	Rid[0].usUsagePage = (USHORT)0x01;	// HID_USAGE_PAGE_GENERIC;
	Rid[0].usUsage = (USHORT)0x02;	// HID_USAGE_GENERIC_MOUSE;
	Rid[0].dwFlags = 0;
	Rid[0].hwndTarget = m_hwnd;
	if (FALSE == (RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]))))	// Cast between semantically different integer types : a Boolean type to HRESULT.
	{
		OutputDebugString("Failed to register raw input device!");
	}

	// get devices and print info
	//-----------------------------------------------------
	UINT numDevices = 0;
	GetRawInputDeviceList(
		NULL, &numDevices, sizeof(RAWINPUTDEVICELIST));
	if (numDevices == 0) return;

	std::vector<RAWINPUTDEVICELIST> deviceList(numDevices);
	GetRawInputDeviceList(
		&deviceList[0], &numDevices, sizeof(RAWINPUTDEVICELIST));

	std::vector<wchar_t> deviceNameData;
	std::wstring deviceName;
	for (UINT i = 0; i < numDevices; ++i)
	{
		const RAWINPUTDEVICELIST& device = deviceList[i];
		if (device.dwType == RIM_TYPEMOUSE)
		{
			char info[1024];
			sprintf_s(info, "Mouse: Handle=0x%08p\n", device.hDevice);
			OutputDebugString(info);

			UINT dataSize = 0;
			GetRawInputDeviceInfo(
				device.hDevice, RIDI_DEVICENAME, nullptr, &dataSize);
			if (dataSize)
			{
				deviceNameData.resize(dataSize);
				UINT result = GetRawInputDeviceInfo(
					device.hDevice, RIDI_DEVICENAME, &deviceNameData[0], &dataSize);
				if (result != UINT_MAX)
				{
					deviceName.assign(deviceNameData.begin(), deviceNameData.end());

					char info[1024];
					std::string ndeviceName(deviceName.begin(), deviceName.end());
					sprintf_s(info, "  Name=%s\n", ndeviceName.c_str());
					OutputDebugString(info);
				}
			}

			RID_DEVICE_INFO deviceInfo;
			deviceInfo.cbSize = sizeof deviceInfo;
			dataSize = sizeof deviceInfo;
			UINT result = GetRawInputDeviceInfo(
				device.hDevice, RIDI_DEVICEINFO, &deviceInfo, &dataSize);
			if (result != UINT_MAX)
			{
#ifdef _DEBUG
				assert(deviceInfo.dwType == RIM_TYPEMOUSE);
#endif
				char info[1024];
				sprintf_s(info,
					"  Id=%u, Buttons=%u, SampleRate=%u, HorizontalWheel=%s\n",
					deviceInfo.mouse.dwId,
					deviceInfo.mouse.dwNumberOfButtons,
					deviceInfo.mouse.dwSampleRate,
					deviceInfo.mouse.fHasHorizontalWheel ? "1" : "0");
				OutputDebugString(info);
			}
		}
	}
}

//-----------------------------------------------------------------------------
// The WndProc function is where windows sends its messages to. You'll notice 
// we tell windows the name of it when we initialize the window class with 
// wc.lpfnWndProc = WndProc in the InitializeWindows function above.
LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch (umessage)
	{
	// Check if the window is being destroyed.
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}

	// Check if the window is being closed.
	case WM_CLOSE:
	{
		PostQuitMessage(0);
		return 0;
	}

	// All other messages pass to the message handler in the system class.
	default:
	{
		return gp_appHandle->MessageHandler(hwnd, umessage, wparam, lparam);
	}
	}
}
