#pragma once

#include "structures.h"
#include <map>
#include <vector>
#include <string>

class TextureManager
{
private:
	UINT textureColorCount;
	UINT textureNormalCount;
	UINT textureSpecularCount;
	std::vector<std::string> texturesColor;
	std::vector<std::string> texturesNormal;
	std::vector<std::string> texturesSpecular;
	std::map<std::string, UINT> textureColorMap;
	std::map<std::string, UINT> textureNormalMap;
	std::map<std::string, UINT> textureSpecularMap;

public:
	TextureManager();
	~TextureManager();
	TextureManager(const TextureManager& c);
	TextureManager& operator=(const TextureManager& c);
	VOID Clear();

	const UINT GetTextureColorCount() const;
	const UINT GetTextureNormalCount() const;
	const UINT GetTextureSpecularCount() const;
	const std::vector<std::string> GetTexturesColor() const;
	const std::vector<std::string> GetTexturesNormal() const;
	const std::vector<std::string> GetTexturesSpecular() const;
	const UINT GetTextureColorID(const H2B::MATERIAL2& mat);
	const UINT GetTextureNormalID(const H2B::MATERIAL2& mat);
	const UINT GetTextureSpecularID(const H2B::MATERIAL2& mat);
	const std::string GetTextureColor(const UINT index) const;
	const std::string GetTextureNormal(const UINT index) const;
	const std::string GetTextureSpecular(const UINT index) const;
};