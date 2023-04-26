#include "wtypes.h"
#include <fstream>
#include <limits>

#include "Settings.h"

Settings::Settings()
{ 
	RECT desktop;
	const HWND hDesktop = GetDesktopWindow();
	GetWindowRect(hDesktop, &desktop);
	windowWidth = desktop.right;
	windowHeight = desktop.bottom;
	fullscreen = false;
}

bool Settings::ReadSettings()
{
	std::ifstream file("data/settings.cfg");
	if (!file.is_open())
	{
		Settings();
		return false;
	}

	std::string identifier;
	while (!file.eof())
	{
		file >> identifier;

		// Skip comment
		if (identifier[0] == '/' && identifier[1] == '/')
		{
			continue;
		}

		if (identifier == "width")
			file >> windowWidth;
		else if (identifier == "height")
			file >> windowHeight;
		else if (identifier == "fullscreen")
			file >> (bool)fullscreen;
		else
		{
			Settings();
			return false;
		}
	}
	return true;
}

int Settings::GetWindowWidth()
{
	return windowWidth;
}

int Settings::GetWindowHeight()
{
	return windowHeight;
}

bool Settings::GetFullscreenMode()
{
	return fullscreen;
}

