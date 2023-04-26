#pragma once

#include <string>

class Settings
{
	private:
		int windowWidth, windowHeight;
		bool fullscreen;
	public:
		Settings();

		bool ReadSettings();

		int GetWindowWidth();
		int GetWindowHeight();
		bool GetFullscreenMode();
};