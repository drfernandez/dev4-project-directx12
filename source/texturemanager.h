#pragma once

#include <Windows.h>
#include "structures.h"
#include <map>

class TextureManager
{
public:
	static TextureManager* GetInstance();
	void Initialize();
	void Shutdown();
	UINT GetTextureID_2D(const H2B::MATERIAL2& material);
	UINT GetTextureID_3D(const H2B::MATERIAL2& material);
	BOOL IsTexture(const H2B::MATERIAL2& material);

private:
	TextureManager();
	~TextureManager();
	TextureManager(const TextureManager& c);
	TextureManager& operator=(const TextureManager& c);

	std::map<std::string, UINT> texture_2D;
	std::map<std::string, UINT> texture_3D;
	std::vector<H2B::MATERIAL2> material_2D;
	std::vector<H2B::MATERIAL2> material_3D;

	void Clear();
};