#pragma once

#include <string>
#include <Windows.h>

class WinWindow
{
	private:
		HWND hwnd;
	public:
		WinWindow();
		~WinWindow();

		bool Create(std::string windowName, int windowWidth, int windowHeight, int windowPosX, int windowPosY, WNDPROC windowProc);
		HWND GetHWND();
};