#pragma once

#define GATEWARE_ENABLE_MATH // Enables all Math Libraries

#include "structures.h"
#include <map>
#include <vector>
#include <fstream>
#include "materialmanager.h"
#include "texturemanager.h"
#include "frustum.h"

class Level
{
private:
	std::ifstream input;
	const UINT COLOR_FLAG = 0x00000001u;
	const UINT NORMAL_FLAG = 0x00000002u;
	const UINT SPECULAR_FLAG = 0x00000004u;

	BOOL FileExists(std::string file);
	std::string GetFileName(std::string file);
	GW::MATH::GMATRIXF ReadMatrixData();
	BOOL LoadH2B(const std::string& h2bFilePath, H2B::INSTANCED_MESH& mesh);
	void LoadMeshFromFile();
	void LoadLightFromFile();
	void LoadCameraFromFile();
	BOOL IsUniqueMesh(const std::map<std::string, H2B::INSTANCED_MESH>& container, const std::string& assetName);
	GW::MATH::GAABBMMF GenerateAABB(const std::vector<H2B::VERTEX>& verts);

public:
	std::string name;
	GW::MATH::GMATRIXF camera;
	FLOAT aspectRatio;
	UINT vertex_count;
	UINT index_count;
	std::vector<H2B::VERTEX> vertices;
	std::vector<UINT> indices;
	std::map<std::string, H2B::INSTANCED_MESH> uniqueMeshes;
	std::map<std::string, H2B::INSTANCED_MESH> uniqueSkyboxes;
	std::vector<H2B::LIGHT> uniqueLights;
	std::vector<GW::MATH::GMATRIXF> instanceData;
	MaterialManager materials;
	TextureManager textures;

	Frustum frustum;
	std::map<std::string, H2B::INSTANCED_MESH> culledUniqueMeshes;
	std::vector<GW::MATH::GMATRIXF> culledInstanceData;
	std::vector<H2B::LIGHT> culledUniqueLights;

	Level();
	~Level();
	BOOL LoadLevel(const std::string& filepath);
	void FrustumCull();
	void Clear();
};