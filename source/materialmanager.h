#pragma once

#include "structures.h"
#include <map>
#include <string>

class MaterialManager
{
public:
	static MaterialManager* GetInstance();

	VOID Initialize();
	VOID Shutdown();
	UINT GetMaterialID(const H2B::MATERIAL2& mat);
	H2B::ATTRIBUTES GetMaterial(const UINT index);
	VOID Clear();

	UINT material_count;
	std::vector<H2B::ATTRIBUTES> materials;
	std::map<std::string, UINT> materialMap;

private:
	MaterialManager();
	~MaterialManager();
	MaterialManager(const MaterialManager& c);
	MaterialManager& operator=(const MaterialManager& c);

};
