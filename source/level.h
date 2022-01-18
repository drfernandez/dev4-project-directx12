#pragma once

#include "structures.h"
#include <map>
#include <vector>
#include <fstream>

class Level
{
private:
	std::ifstream input;

	BOOL FileExists(std::string file);
	std::string GetFileName(std::string file);
	GW::MATH::GMATRIXF ReadMatrixData();
	BOOL LoadH2B(const std::string& h2bFilePath, H2B::INSTANCED_MESH& mesh);
	void LoadMeshFromFile();
	void LoadLightFromFile();
	void LoadCameraFromFile();

public:
	std::string name;
	GW::MATH::GMATRIXF camera;
	UINT vertex_count;
	UINT index_count;
	UINT material_count;
	std::vector<H2B::VERTEX> vertices;
	std::vector<UINT> indices;
	std::vector<H2B::MATERIAL2> materials;
	std::map<std::string, H2B::INSTANCED_MESH> uniqueMeshes;
	std::map<std::string, H2B::INSTANCED_MESH> uniqueSkyboxes;
	std::vector<H2B::LIGHT> uniqueLights;

	Level();
	~Level();
	BOOL LoadLevel(const std::string& filepath);
	void Clear();
};