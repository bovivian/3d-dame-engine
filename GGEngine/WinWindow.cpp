
#include "WinWindow.h"
#include "LogManager.h"

extern LogManager * gLogManager;

WinWindow::WinWindow()
{
	hwnd = NULL;
}

WinWindow::~WinWindow()
{

}

bool WinWindow::Create(std::string windowName, int windowWidth, int windowHeight, int windowPosX, int windowPosY, WNDPROC windowProc)
{
	WNDCLASSEX wc;
	wc.cbClsExtra = 0;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)8;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hIconSm = wc.hIcon;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpfnWndProc = windowProc;
	wc.lpszClassName = windowName.c_str();
	wc.lpszMenuName = NULL;
	wc.style = CS_VREDRAW | CS_HREDRAW;

	if (!RegisterClassEx(&wc))
	{
		gLogManager->AddMessage("ERROR: Failed to register window class!");
		return false;
	}

	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = windowWidth;
	rect.bottom = windowHeight;
	AdjustWindowRectEx(&rect, WS_POPUP, false, WS_EX_APPWINDOW | WS_EX_WINDOWEDGE);

	hwnd = CreateWindowEx(WS_EX_WINDOWEDGE, windowName.c_str(), windowName.c_str(), WS_POPUP, windowPosX, windowPosY, rect.right - rect.left,
		rect.bottom - rect.top, NULL, NULL, GetModuleHandle(NULL), NULL);

	if (hwnd == NULL)
	{
		gLogManager->AddMessage("ERROR: CreateWindowEx() failed!");
		return false;
	}

	ShowWindow(hwnd, SW_HIDE);
	SetFocus(hwnd);
	return true;
}

HWND WinWindow::GetHWND()
{
	return hwnd;
}

