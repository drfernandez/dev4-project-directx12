#include "materialmanager.h"


MaterialManager* MaterialManager::GetInstance()
{
	static MaterialManager instance;
	return &instance;
}

UINT MaterialManager::GetMaterialID(const H2B::MATERIAL2& mat)
{
	return 0;
}

H2B::MATERIAL2 MaterialManager::GetMaterial(const UINT index)
{
	H2B::MATERIAL2 m = H2B::MATERIAL2();
	return m;
}

MaterialManager::MaterialManager()
{
	Clear();
}

MaterialManager::~MaterialManager()
{
	Clear();
}

MaterialManager::MaterialManager(const MaterialManager& c)
{
	*this = c;
}

MaterialManager& MaterialManager::operator=(const MaterialManager& c)
{
	// private function, will not be called
	if (this != &c)
	{
	}
	return *this;
}

void MaterialManager::Clear()
{
	materials.clear();
	materialMap.clear();
}
