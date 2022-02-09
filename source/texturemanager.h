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
	std::vector<std::string> texturesColor;
	std::vector<std::string> texturesNormal;
	std::map<std::string, UINT> textureColorMap;
	std::map<std::string, UINT> textureNormalMap;

public:
	TextureManager();
	~TextureManager();
	TextureManager(const TextureManager& c);
	TextureManager& operator=(const TextureManager& c);
	VOID Clear();

	const UINT GetTextureColorCount() const;
	const UINT GetTextureNormalCount() const;
	const std::vector<std::string> GetTexturesColor() const;
	const std::vector<std::string> GetTexturesNormal() const;
	const UINT GetTextureColorID(const H2B::MATERIAL2& mat);
	const UINT GetTextureNormalID(const H2B::MATERIAL2& mat);
	const std::string GetTextureColor(const UINT index) const;
	const std::string GetTextureNormal(const UINT index) const;
};