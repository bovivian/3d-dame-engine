#pragma once

#include "Texture.h"

class Material
{
	private:
		Texture * diffuseTexture;
		Texture * materialTexture;
		Texture * normalTexture;
		float metallicOffset;
		float roughnessOffset;
	public:
		Material();
		~Material();

		void SetDiffuseTexture(Texture * texture);
		void SetMaterialTexture(Texture * texture);
		void SetNormalTexture(Texture * texture);
		void SetMetallicOffset(float value);
		void SetRoughnessOffset(float value);
		Texture * GetDiffuseTexture();
		Texture * GetMaterialTexture();
		Texture * GetNormalTexture();
		bool HasNormalMap();
		float GetMetallicOffset();
		float GetRoughnessOffset();
};