#include "materialmanager.h"


MaterialManager* MaterialManager::GetInstance()
{
	// static variable for the singleton
	static MaterialManager instance;
	// return the static variable (instance of the singleton)
	return &instance;
}

VOID MaterialManager::Initialize()
{
	Clear();
}

VOID MaterialManager::Shutdown()
{
	Clear();
}

UINT MaterialManager::GetMaterialID(const H2B::MATERIAL2& mat)
{
	// index variable to return from the function
	UINT index = -1;
	// find the material in the map
	auto iter = materialMap.find(mat.name);
	// if the item was not found
	if (iter == materialMap.end())
	{
		// store the index
		index = materials.size();
		// push into the container
		materials.push_back(mat);
		// insert into the map
		materialMap[mat.name] = index;
		// up the material count
		material_count++;
	}
	// the material was found in the map
	else
	{
		// set the variable to the iter's second
		index = iter->second;
	}
	// return the index for the material
	return index;
}

H2B::MATERIAL2 MaterialManager::GetMaterial(const UINT index)
{
	// variable for the material
	H2B::MATERIAL2 m = H2B::MATERIAL2();
	// check for a valid index ? if yes get the material : if no return a default value
	m = (index < materials.size()) ? materials[index] : m;
	return m;
}

MaterialManager::MaterialManager()
{
	// Clear the variables
	Clear();
}

MaterialManager::~MaterialManager()
{
	// Clear the variables
	Clear();
}

MaterialManager::MaterialManager(const MaterialManager& c)
{
	// call the assignment operator
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

VOID MaterialManager::Clear()
{
	// set the count to 0
	material_count = 0;
	// clear the vector
	materials.clear();
	// clear the map
	materialMap.clear();
}
