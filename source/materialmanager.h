#pragma once

#include "structures.h"
#include <map>
#include <string>

class MaterialManager
{
public:
	static MaterialManager* GetInstance();

	UINT GetMaterialID(const H2B::MATERIAL2& mat);
	H2B::MATERIAL2 GetMaterial(const UINT index);

private:
	std::vector<H2B::MATERIAL2> materials;
	std::map<std::string, UINT> materialMap;

	MaterialManager();
	~MaterialManager();
	MaterialManager(const MaterialManager& c);
	MaterialManager& operator=(const MaterialManager& c);
	void Clear();

};
