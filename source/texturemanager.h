#pragma once

#include "structures.h"
#include <map>
#include <vector>
#include <string>

class TextureManager
{
private:
	UINT texture_count;
	std::vector<std::string> textures;
	std::map<std::string, UINT> textureMap;

public:
	TextureManager();
	~TextureManager();
	TextureManager(const TextureManager& c);
	TextureManager& operator=(const TextureManager& c);
	VOID Clear();

	const UINT GetTextureCount() const;
	const std::vector<std::string> GetTextures() const;
	const UINT GetTextureID(const H2B::MATERIAL2& mat);
	const std::string GetTexture(const UINT index) const;
};