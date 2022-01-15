#pragma once

#include "structures.h"
#include <map>
#include <vector>
#include <fstream>
#include "texturemanager.h"

class Level
{
private:
	std::ifstream input;
	TextureManager* tm;

	BOOL FileExists(std::string file);
	std::string GetFileName(std::string file);
	GW::MATH::GMATRIXF ReadMatrixData();
	BOOL LoadH2B(const std::string& h2bFilePath, H2B::INSTANCED_MESH& mesh);
	//UINT Find2DMaterialIndex(const H2B::MATERIAL2& material);
	//UINT Find3DMaterialIndex(const H2B::MATERIAL2& material);
	void LoadMeshFromFile();
	void LoadLightFromFile();
	void LoadCameraFromFile();

public:
	std::string name;
	UINT vertex_count;
	UINT index_count;
	UINT material_count;
	std::vector<H2B::VERTEX> vertices;
	std::vector<UINT> indices;
	//std::vector<H2B::MATERIAL2> materials2D;
	//std::vector<H2B::MATERIAL2> materials3D;
	std::map<std::string, H2B::INSTANCED_MESH> uniqueMeshes;
	std::map<std::string, H2B::INSTANCED_MESH> uniqueSkyboxes;
	//std::map<std::string, UINT> uniqueMaterials2D;
	//std::map<std::string, UINT> uniqueMaterials3D;
	std::vector<H2B::LIGHT> uniqueLights;
	GW::MATH::GMATRIXF camera;

	Level();
	~Level();
	bool LoadLevel(const std::string& filepath);
	void Clear();
};