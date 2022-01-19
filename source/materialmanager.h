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
	H2B::MATERIAL2 GetMaterial(const UINT index);

	UINT material_count;
	std::vector<H2B::MATERIAL2> materials;
	std::map<std::string, UINT> materialMap;

private:
	MaterialManager();
	~MaterialManager();
	MaterialManager(const MaterialManager& c);
	MaterialManager& operator=(const MaterialManager& c);
	VOID Clear();

};
