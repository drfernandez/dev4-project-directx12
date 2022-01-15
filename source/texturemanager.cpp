#include "texturemanager.h"


TextureManager::TextureManager()
{
	Clear();
}

TextureManager::~TextureManager()
{
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
		this->Clear();
		this->material_2D.resize(c.material_2D.size());
		this->material_3D.resize(c.material_3D.size());
		memcpy_s(this->material_2D.data(),
			sizeof(H2B::MATERIAL2) * c.material_2D.size(),
			c.material_2D.data(),
			sizeof(H2B::MATERIAL2) * c.material_2D.size());
		memcpy_s(this->material_3D.data(),
			sizeof(H2B::MATERIAL2) * c.material_3D.size(),
			c.material_3D.data(),
			sizeof(H2B::MATERIAL2) * c.material_3D.size());
	}
	return *this;
}

void TextureManager::Clear()
{
	texture_2D.clear();
	texture_3D.clear();
	material_2D.clear();
	material_3D.clear();
}

TextureManager* TextureManager::GetInstance()
{
	static TextureManager instance; 
	return &instance;
}

void TextureManager::Initialize()
{
	Clear();
}

void TextureManager::Shutdown()
{
	Clear();
}

UINT TextureManager::GetTextureID_2D(const H2B::MATERIAL2& material)
{
	unsigned int index = -1;
	auto iterator = texture_2D.find(material.name);
	if (iterator == texture_2D.end())
	{
		index = texture_2D.size();
		texture_2D[material.name] = index;
		material_2D.push_back(material);
	}
	else
	{
		index = iterator->second;
	}
	return index;
}

UINT TextureManager::GetTextureID_3D(const H2B::MATERIAL2& material)
{
	unsigned int index = -1;
	auto iterator = texture_3D.find(material.name);
	if (iterator == texture_3D.end())
	{
		index = texture_3D.size();
		texture_3D[material.name] = index;
		material_3D.push_back(material);
	}
	else
	{
		index = iterator->second;
	}
	return index;
}

BOOL TextureManager::IsTexture(const H2B::MATERIAL2& material)
{
	return !material.map_Kd.empty();
}
