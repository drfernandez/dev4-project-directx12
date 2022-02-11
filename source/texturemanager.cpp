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
		this->textureColorCount = c.textureColorCount;
		this->texturesColor = c.texturesColor;
		this->textureColorMap = c.textureColorMap;
	}
	return *this;
}

VOID TextureManager::Clear()
{
	// set the count to 0
	textureColorCount = 0;
	textureNormalCount = 0;
	// clear the vector
	texturesColor.clear();
	texturesNormal.clear();
	// clear the map
	textureColorMap.clear();
	textureNormalMap.clear();
}

const UINT TextureManager::GetTextureColorCount() const
{
	// return the texture count
	return textureColorCount;
}

const UINT TextureManager::GetTextureNormalCount() const
{
	return textureNormalCount;
}

const std::vector<std::string> TextureManager::GetTexturesColor() const
{
	// return the vector of texture names
	return texturesColor;
}

const std::vector<std::string> TextureManager::GetTexturesNormal() const
{
	return texturesNormal;
}

const UINT TextureManager::GetTextureColorID(const H2B::MATERIAL2& mat)
{
	// index variable to return from the function
	UINT index = 0;
	if (!mat.map_Kd.empty())
	{
		// find the material in the map
		auto iter = textureColorMap.find(mat.map_Kd);
		// if the item was not found
		if (iter == textureColorMap.end())
		{
			// store the index
			index = texturesColor.size();
			// push into the container
			texturesColor.push_back(mat.map_Kd);
			// insert into the map
			textureColorMap[mat.map_Kd] = index;
			// up the material count
			textureColorCount++;
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

const UINT TextureManager::GetTextureNormalID(const H2B::MATERIAL2& mat)
{
	// index variable to return from the function
	UINT index = 0;
	if (!mat.map_Ns.empty())
	{
		// find the material in the map
		auto iter = textureNormalMap.find(mat.map_Ns);
		// if the item was not found
		if (iter == textureNormalMap.end())
		{
			// store the index
			index = texturesNormal.size();
			// push into the container
			texturesNormal.push_back(mat.map_Ns);
			// insert into the map
			textureNormalMap[mat.map_Ns] = index;
			// up the material count
			textureNormalCount++;
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

const std::string TextureManager::GetTextureColor(const UINT index) const
{
	std::string v = std::string();
	v = (index < texturesColor.size()) ? texturesColor[index] : v;
	return v;
}

const std::string TextureManager::GetTextureNormal(const UINT index) const
{
	std::string v = std::string();
	v = (index < texturesNormal.size()) ? texturesNormal[index] : v;
	return v;
}
