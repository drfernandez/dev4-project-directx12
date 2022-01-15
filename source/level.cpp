#include "level.h"

Level::Level()
{
	Clear();
	tm = TextureManager::GetInstance();
	tm->Initialize();
}

Level::~Level()
{
	Clear();
	tm->Shutdown();
}

bool Level::LoadLevel(const std::string& filepath)
{
	return false;
}

void Level::Clear()
{
	if (input.is_open()) input.close();
	name.clear();
	vertex_count = 0;
	index_count = 0;
	material_count = 0;
	vertices.clear();
	indices.clear();
	uniqueMeshes.clear();
	uniqueSkyboxes.clear();
	uniqueLights.clear();
	camera = GW::MATH::GIdentityMatrixF;
}

BOOL Level::FileExists(std::string file)
{
	std::ifstream in;
	in.open(file, std::ios_base::in);
	BOOL result = in.is_open();
	in.close();
	return result;
}

std::string Level::GetFileName(std::string file)
{
	std::string tokenize = file;
	std::string delim = "/\\";
	std::string submit = "";
	size_t first = 0;
	while (first < tokenize.size())
	{
		// find first delimiter
		size_t second = tokenize.find_first_of(delim, first);
		// check to see if it's the end of the string
		if (second == std::string::npos)
		{
			// set the second location to the size of the entire string
			second = tokenize.size();
		}
		// store the value of the tokenized string
		submit = tokenize.substr(first, second - first);
		// adjust the location to search
		first = second + 1;
	}

	// Remove extension
	delim = ".";
	first = submit.find_first_of(delim, 0);
	submit = submit.substr(0, first);
	return submit;
}

GW::MATH::GMATRIXF Level::ReadMatrixData()
{
	GW::MATH::GMATRIXF matrix = {};
	char buffer[256] = {};
	input.getline(buffer, 256, '('); // read up to the start of the matrix data

	input.getline(buffer, 256, ','); // x
	matrix.row1.x = std::stof(buffer);
	input.getline(buffer, 256, ','); // y
	matrix.row1.y = std::stof(buffer);
	input.getline(buffer, 256, ','); // z
	matrix.row1.z = std::stof(buffer);
	input.getline(buffer, 256, ')'); // w
	matrix.row1.w = std::stof(buffer);

	input.getline(buffer, 256);
	input.getline(buffer, 256, '(');

	input.getline(buffer, 256, ','); // x
	matrix.row2.x = std::stof(buffer);
	input.getline(buffer, 256, ','); // y
	matrix.row2.y = std::stof(buffer);
	input.getline(buffer, 256, ','); // z
	matrix.row2.z = std::stof(buffer);
	input.getline(buffer, 256, ')'); // w
	matrix.row2.w = std::stof(buffer);

	input.getline(buffer, 256);
	input.getline(buffer, 256, '(');

	input.getline(buffer, 256, ','); // x
	matrix.row3.x = std::stof(buffer);
	input.getline(buffer, 256, ','); // y
	matrix.row3.y = std::stof(buffer);
	input.getline(buffer, 256, ','); // z
	matrix.row3.z = std::stof(buffer);
	input.getline(buffer, 256, ')'); // w
	matrix.row3.w = std::stof(buffer);

	input.getline(buffer, 256);
	input.getline(buffer, 256, '(');

	input.getline(buffer, 256, ','); // x
	matrix.row4.x = std::stof(buffer);
	input.getline(buffer, 256, ','); // y
	matrix.row4.y = std::stof(buffer);
	input.getline(buffer, 256, ','); // z
	matrix.row4.z = std::stof(buffer);
	input.getline(buffer, 256, ')'); // w
	matrix.row4.w = std::stof(buffer);

	input.getline(buffer, 256);

	return matrix;
}

BOOL Level::LoadH2B(const std::string& h2bFilePath, H2B::INSTANCED_MESH& instancedMesh)
{
	BOOL success = false;
	H2B::Parser p;
	if (p.Parse(h2bFilePath.c_str()))
	{
		for (const auto& material : p.materials)
		{
			H2B::MATERIAL2 mat = H2B::MATERIAL2(material);
			BOOL IsSkybox = (mat.name.compare("Skybox_Texture") != 0);
			INT id = (IsSkybox) ? tm->GetTextureID_3D(mat) : tm->GetTextureID_2D(mat);
		}

		for (const auto& mesh : p.meshes)
		{
			H2B::MESH2 m = H2B::MESH2(mesh);

			m.drawInfo.indexOffset += index_count;
			bool IsCubeMap = (strcmp(p.materials[m.materialIndex].name, "Skybox_Texture") == 0);
			H2B::MATERIAL2 material = H2B::MATERIAL2(p.materials[m.materialIndex]);
			m.materialIndex = (!IsCubeMap) ? tm->GetTextureID_2D(material) : tm->GetTextureID_3D(material);
			m.hasColorTexture = (tm->IsTexture(material)) ? 1 : 0;
			instancedMesh.subMeshes.push_back(m);
		}

		instancedMesh.vertexOffset = vertex_count;

		for (const auto& vertex : p.vertices)
		{
			vertices.push_back(vertex);
		}
		for (const auto& index : p.indices)
		{
			indices.push_back(index);
		}

		vertex_count += p.vertexCount;
		index_count += p.indexCount;
		material_count += p.materialCount;
		success = true;
	}

	return success;
}

void Level::LoadMeshFromFile()
{
	char buffer[256];
	// get the mesh name
	input.getline(buffer, 256);
	// read matrix data and store
	GW::MATH::GMATRIXF world = ReadMatrixData();

	std::string assetName = GetFileName(buffer);
	std::string path = "../assets/" + assetName + ".h2b";

	H2B::INSTANCED_MESH instancedMesh = {};
	if (FileExists(path) && LoadH2B(path, instancedMesh))
	{
		instancedMesh.meshName = assetName;
		instancedMesh.numInstances = 1;
		instancedMesh.matrices.push_back(world);
	}

	auto& container = (assetName == "Skybox") ? uniqueSkyboxes : uniqueMeshes;
	auto containerIter = container.find(assetName);
	if (containerIter == container.end())
	{
		container[assetName] = instancedMesh;
	}
	else
	{
		container[assetName].numInstances += 1;
		container[assetName].matrices.push_back(world);
	}	
}

void Level::LoadLightFromFile()
{
	char buffer[256] = {};
	input.getline(buffer, 256);
	GW::MATH::GMATRIXF information = ReadMatrixData();
	if (strcmp(buffer, "Point") == 0)
	{
		information.row1.w = 1.0f;
	}
	else if (strcmp(buffer, "Spot") == 0)
	{
		information.row1.w = 2.0f;
	}
	H2B::LIGHT l = H2B::LIGHT(
		information.row1,
		information.row2,
		information.row3,
		information.row4);
	uniqueLights.push_back(l);
}

void Level::LoadCameraFromFile()
{
	char buffer[256] = {};
	input.getline(buffer, 256);
	GW::MATH::GMATRIXF information = ReadMatrixData();
	camera = information;
}
