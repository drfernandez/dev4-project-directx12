#include "texturemanager.h"


TextureManager::TextureManager()
{
	// Clear the variables
	Clear();
}

TextureManager::~TextureManager()
{
	// Clear the variables
	Clear();
}

TextureManager::TextureManager(const TextureManager& c)
{
	*this = c;
}

TextureManager& TextureManager::operator=(const TextureManager& c)
{
	if (this != &c)
	{
		this->texture_count = c.texture_count;
		this->textures = c.textures;
		this->textureMap = c.textureMap;
	}
	return *this;
}

VOID TextureManager::Clear()
{
	// set the count to 0
	texture_count = 0;
	// clear the vector
	textures.clear();
	// clear the map
	textureMap.clear();
}

const UINT TextureManager::GetTextureCount() const
{
	// return the texture count
	return texture_count;
}

const std::vector<std::string> TextureManager::GetTextures() const
{
	// return the vector of texture names
	return textures;
}

const UINT TextureManager::GetTextureID(const H2B::MATERIAL2& mat)
{
	// index variable to return from the function
	UINT index = -1;
	if (!mat.map_Kd.empty())
	{
		// find the material in the map
		auto iter = textureMap.find(mat.map_Kd);
		// if the item was not found
		if (iter == textureMap.end())
		{
			// store the index
			index = textures.size();
			// push into the container
			textures.push_back(mat.map_Kd);
			// insert into the map
			textureMap[mat.map_Kd] = index;
			// up the material count
			texture_count++;
		}
		// the material was found in the map
		else
		{
			// set the variable to the iter's second
			index = iter->second;
		}
	}
	// return the index for the material
	return index;
}

const std::string TextureManager::GetTexture(const UINT index) const
{
	std::string v = std::string();
	v = (index < textures.size()) ? textures[index] : v;
	return v;
}
