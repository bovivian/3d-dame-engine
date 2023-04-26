#pragma once

#include "glm.hpp"

class FrustumCuller
{
	private:
		glm::vec4 planes[6];
	public:
		FrustumCuller();

		void BuildFrustum(glm::mat4 viewProjMatrix);
		bool IsInsideFrustum(class Model * model);
};