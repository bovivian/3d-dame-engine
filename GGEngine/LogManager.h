#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <glm.hpp>

class LogManager
{
	private:
		std::ofstream file;
	public:
		bool Init();
		~LogManager();
		void AddMessage(std::string msg);
		void PrintVector(glm::vec2 vec);
		void PrintVector(glm::vec3 vec);
		void PrintVector(glm::vec4 vec);
		void PrintValue(float val);
};
