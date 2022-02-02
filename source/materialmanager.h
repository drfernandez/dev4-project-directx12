#pragma once

#include "structures.h"
#include <map>
#include <vector>
#include <string>

class MaterialManager
{
private:
	UINT material_count;
	std::vector<H2B::ATTRIBUTES> materials;
	std::map<std::string, UINT> materialMap;

public:
	MaterialManager();
	~MaterialManager();
	MaterialManager(const MaterialManager& c);
	MaterialManager& operator=(const MaterialManager& c);
	VOID Clear();

	const UINT GetMaterialCount() const;
	const std::vector<H2B::ATTRIBUTES> GetMaterials() const;
	const UINT GetMaterialID(const H2B::MATERIAL2& mat);
	const H2B::ATTRIBUTES GetMaterial(const UINT index) const;
};
