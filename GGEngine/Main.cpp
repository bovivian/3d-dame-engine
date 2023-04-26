#pragma once
#include <iostream>
#include <string>

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>

#include "StdInc.h"
#include "LogManager.h"
#include "WinWindow.h"
#include "Settings.h"
#include "VulkanInterface.h"
#include "Input.h"
#include "Timer.h"
#include "SceneManager.h"


LRESULT CALLBACK WinWindowProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);

LogManager * gLogManager;
Settings * gSettings;
Input * gInput;
Timer * gTimer;
SceneManager * sceneManager = new SceneManager();

bool gProgramRunning = true;
bool inGame = true;

int main()
{
	// Console window
	HWND consoleWindow = GetConsoleWindow();
	SetWindowPos(consoleWindow, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER); // to hide console add attribute: SWP_HIDEWINDOW

	// Log manager
	gLogManager = new LogManager();
	if (!gLogManager->Init())
	{
		std::cout << "ERROR: Failed to init LogManager!" << std::endl;
		return 1;
	}
	gLogManager->AddMessage(std::string(PROGRAM_IDENTIFIER) + " started!");

	// Settings
	gSettings = new Settings();
	if (!gSettings->ReadSettings())
		gLogManager->AddMessage("WARNING: Couldn't read settings.cfg! Using defaults...");
	else
		gLogManager->AddMessage("SUCCESS: Loaded settings.cfg!");

	// Render window
	WinWindow * window = new WinWindow();
	if (!window->Create(PROGRAM_NAME, gSettings->GetWindowWidth(), gSettings->GetWindowHeight(), 0, 0, WinWindowProc))
	{
		gLogManager->AddMessage("ERROR: Failed to init render window!");
		THROW_ERROR();
	}
	
	// Input
	gInput = new Input();
	if (!gInput->Init(window->GetHWND()))
	{
		gLogManager->AddMessage("ERROR: Failed to init input interface!");
		return false;
	}

	//Vulkan interface
	VulkanInterface * vulkan = new VulkanInterface();
	if (!vulkan->Init(window->GetHWND()))
	{
		gLogManager->AddMessage("ERROR: Failed to init vulkan interface!");
		THROW_ERROR();
	}
	gLogManager->AddMessage("SUCCESS: Vulkan interface initialized!");

	// Timer
	gTimer = new Timer();
	if (!gTimer->Init())
	{
		gLogManager->AddMessage("ERROR: Failed to init timer!");
		return false;
	}

	// Scene manager
	
	if (!sceneManager->Init(vulkan))
	{
		gLogManager->AddMessage("ERROR: Failed to init scene manager!");
		THROW_ERROR();
	}
	
	// Main loop
	MSG msg;
	while (sceneManager->GetProgramRunning())
	{	
		while (PeekMessage(&msg, window->GetHWND(), 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			
			if(msg.message == WM_INPUT)
			{
				UINT dwSize;
				GetRawInputData((HRAWINPUT)msg.lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
				LPBYTE lpb = new BYTE[dwSize];

				GetRawInputData((HRAWINPUT)msg.lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER));

				RAWINPUT * raw = (RAWINPUT*)lpb;

				if (raw->header.dwType == RIM_TYPEMOUSE) {
					gInput->InputHandler_SetCursorRelatives((int)raw->data.mouse.lLastX, (int)raw->data.mouse.lLastY);
					
					if (raw->data.mouse.usButtonFlags == RI_MOUSE_LEFT_BUTTON_UP) {
						gInput->InputHandler_SetKeyUp(MOUSE_KEY_L);
					}

					if (raw->data.mouse.usButtonFlags == RI_MOUSE_LEFT_BUTTON_DOWN)
					{
						gInput->InputHandler_SetKeyDown(MOUSE_KEY_L);
					}
						
					if (raw->data.mouse.usButtonFlags == RI_MOUSE_RIGHT_BUTTON_UP) {
						gInput->InputHandler_SetKeyUp(MOUSE_KEY_R);
					}

					if (raw->data.mouse.usButtonFlags == RI_MOUSE_RIGHT_BUTTON_DOWN)
					{
						gInput->InputHandler_SetKeyDown(MOUSE_KEY_R);
					}

				}
				else if (raw->header.dwType = RIM_TYPEKEYBOARD)
				{
					if (raw->data.keyboard.Flags == RI_KEY_BREAK)
						gInput->InputHandler_SetKeyUp(raw->data.keyboard.VKey);
					if (raw->data.keyboard.Flags == RI_KEY_MAKE)
						gInput->InputHandler_SetKeyDown(raw->data.keyboard.VKey);
				}
			}
		}
		if (inGame) {
			gTimer->Update();
			gInput->Update();
			sceneManager->Render(vulkan);
			if (sceneManager->GetLoggedIn())
			{
				ShowWindow(window->GetHWND(), SW_SHOW);
				ShowWindow(consoleWindow, SW_HIDE);
			}
				

			if (gInput->WasKeyPressed(KEYBOARD_KEY_R))
			{
				char msg[64];
				sprintf(msg, "FPS: %d FRAME TIME: %f BENCH TIME: %f", gTimer->GetFPS(), gTimer->GetDelta(), gTimer->GetBenchmarkResult());
				gLogManager->AddMessage(msg);
			}
		}
	}

	vkDeviceWaitIdle(vulkan->GetVulkanDevice()->GetDevice());

	gLogManager->AddMessage("Unloading...");
	SAFE_DELETE(gTimer);
	SAFE_UNLOAD(sceneManager, vulkan);
	SAFE_DELETE(vulkan);
	SAFE_DELETE(gInput);
	SAFE_DELETE(window);
	SAFE_DELETE(gSettings);
	SAFE_DELETE(gLogManager);

	return 0;
}

LRESULT CALLBACK WinWindowProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg)
	{
		case WM_CLOSE:
			sceneManager->SetProgramRunning(false);
		break;
		case WM_DESTROY:
			sceneManager->SetProgramRunning(false);
		break;
		case WM_SETFOCUS:
		{
			RECT rect;
			GetClientRect(hwnd, &rect);
			POINT p1 = { rect.left, rect.top };
			POINT p2 = { rect.right, rect.bottom };
			ClientToScreen(hwnd, &p1);
			ClientToScreen(hwnd, &p2);
			SetRect(&rect, p1.x, p1.y, p2.x, p2.y);
			ClipCursor(&rect);
			ShowCursor(FALSE);
		}
		break;
		case WM_KILLFOCUS:
		{
			ClipCursor(NULL);
			ShowCursor(TRUE);
		}
		break;
	}

	return DefWindowProc(hwnd, umsg, wparam, lparam);
}