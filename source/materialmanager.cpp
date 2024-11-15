#include "materialmanager.h"


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
	if (this != &c)
	{
		this->material_count = c.material_count;
		this->materials = c.materials;
		this->materialMap = c.materialMap;
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

const UINT MaterialManager::GetMaterialCount() const
{
	// return the material count
	return material_count;
}

const std::vector<H2B::ATTRIBUTES> MaterialManager::GetMaterials() const
{
	// return the vector of materials
	return materials;
}

const UINT MaterialManager::GetMaterialID(const H2B::MATERIAL2& mat)
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
		materials.push_back(mat.attrib);
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

const H2B::ATTRIBUTES MaterialManager::GetMaterial(const UINT index) const
{
	// variable for the material
	H2B::ATTRIBUTES a = H2B::ATTRIBUTES();
	// check for a valid index ? if yes get the material : if no return a default value
	a = (index < materials.size()) ? materials[index] : a;
	return a;
}