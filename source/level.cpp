#include "level.h"

Level::Level()
{
	Clear();
}

Level::~Level()
{
	Clear();
}

BOOL Level::LoadLevel(const std::string& filepath)
{
	Clear();
	std::string name = "../levels/" + GetFileName(filepath) + ".txt";
	input.open(name.c_str(), std::ios_base::in);
	if (!input.is_open())
	{
		return FALSE;
	}
	this->name = GetFileName(filepath);
	while (!input.eof())
	{
		CHAR buffer[256] = { 0 };
		input.getline(buffer, 256);
		if (strcmp(buffer, "MESH") == 0)
		{
			LoadMeshFromFile();
		}
		else if (strcmp(buffer, "LIGHT") == 0)
		{
			LoadLightFromFile();
		}
		else if (strcmp(buffer, "CAMERA") == 0)
		{
			LoadCameraFromFile();
		}
	}
	input.close();

	UINT meshID = 0;
	for (auto& mesh : uniqueMeshes)
	{
		mesh.second.meshIndex = meshID;
		instanceData.insert(instanceData.end(), mesh.second.matrices.begin(), mesh.second.matrices.end());
		meshID = instanceData.size();
	}
	for (auto& skybox : uniqueSkyboxes)
	{
		skybox.second.meshIndex = meshID;
		meshID += skybox.second.matrices.size();
	}

	return TRUE;
}

void Level::FrustumCull()
{
	culledUniqueMeshes.clear();

	frustum.Create(G_DEGREE_TO_RADIAN_F(85.0f), aspectRatio, 0.1f, 1000.0f, camera);

	for (const auto& mesh : uniqueMeshes)
	{
		H2B::INSTANCED_MESH currentMesh = mesh.second;
		GW::MATH::GAABBMMF aabb = currentMesh.aabb;
		currentMesh.matrices.clear();
		currentMesh.numInstances = 0;

		for (const auto& matrix : mesh.second.matrices)
		{
			GW::MATH::GVector::VectorXMatrixF(currentMesh.aabb.min, matrix, aabb.min);
			GW::MATH::GVector::VectorXMatrixF(currentMesh.aabb.max, matrix, aabb.max);
			if (frustum.CompareAABBToFrustum(aabb))
			{
				currentMesh.matrices.push_back(matrix);
				currentMesh.numInstances += 1;
			}
		}

		if (currentMesh.numInstances > 0)
		{
			culledUniqueMeshes.insert(std::pair<std::string, H2B::INSTANCED_MESH>{ mesh.first, currentMesh });
		}
	}

	culledInstanceData.clear();
	UINT meshID = 0;
	for (auto& mesh : culledUniqueMeshes)
	{
		mesh.second.meshIndex = meshID;
		culledInstanceData.insert(culledInstanceData.end(), mesh.second.matrices.begin(), mesh.second.matrices.end());
		meshID = culledInstanceData.size();
	}
}

void Level::Clear()
{
	if (input.is_open()) input.close();
	name.clear();
	camera = GW::MATH::GIdentityMatrixF;
	aspectRatio = 0.0f;
	vertex_count = 0;
	index_count = 0;
	vertices.clear();
	indices.clear();
	uniqueMeshes.clear();
	uniqueSkyboxes.clear();
	uniqueLights.clear();
	instanceData.clear();
	materials.Clear();
	textures.Clear();
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
	CHAR buffer[4][256] = {};
	int sscanfRetValue = -1;

	input.getline(buffer[0], 256);
	input.getline(buffer[1], 256);
	input.getline(buffer[2], 256);
	input.getline(buffer[3], 256);
	sscanfRetValue = sscanf_s(buffer[0], "<Matrix 4x4 (%f, %f, %f, %f)",	&matrix.row1.x, &matrix.row1.y, &matrix.row1.z, &matrix.row1.w);
	sscanfRetValue = sscanf_s(buffer[1], "            (%f, %f, %f, %f)",	&matrix.row2.x, &matrix.row2.y, &matrix.row2.z, &matrix.row2.w);
	sscanfRetValue = sscanf_s(buffer[2], "            (%f, %f, %f, %f)",	&matrix.row3.x, &matrix.row3.y, &matrix.row3.z, &matrix.row3.w);
	sscanfRetValue = sscanf_s(buffer[3], "            (%f, %f, %f, %f)>",	&matrix.row4.x, &matrix.row4.y, &matrix.row4.z, &matrix.row4.w);

	return matrix;
}

BOOL Level::LoadH2B(const std::string& h2bFilePath, H2B::INSTANCED_MESH& instancedMesh)
{
	// insert materials, vertices, indicies into the level for storage
	BOOL success = FALSE;
	H2B::Parser p;
	if (p.Parse(h2bFilePath.c_str()))
	{
		for (const auto& mesh : p.meshes)
		{
			H2B::MESH2 m = H2B::MESH2(mesh);
			H2B::MATERIAL2 mat = H2B::MATERIAL2(p.materials[m.materialIndex]);
			m.drawInfo.indexOffset += index_count;
			m.materialIndex = materials.GetMaterialID(mat);
			m.diffuseTextureIndex = textures.GetTextureColorID(mat);
			m.normalTextureIndex = textures.GetTextureNormalID(mat);
			m.specularTextureIndex = textures.GetTextureSpecularID(mat);
			m.hasTexture |= (!mat.map_Kd.empty()) ? COLOR_FLAG : 0;
			m.hasTexture |= (!mat.bump.empty()) ? NORMAL_FLAG : 0;
			m.hasTexture |= (!mat.map_Ns.empty()) ? SPECULAR_FLAG : 0;
			instancedMesh.subMeshes.push_back(m);
		}

		instancedMesh.vertexOffset = vertex_count;
		instancedMesh.aabb = GenerateAABB(p.vertices);

		vertices.insert(vertices.end(), p.vertices.begin(), p.vertices.end());
		indices.insert(indices.end(), p.indices.begin(), p.indices.end());

		vertex_count += p.vertexCount;
		index_count += p.indexCount;

		success = TRUE;
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

	auto& container = (assetName == "Skybox") ? uniqueSkyboxes : uniqueMeshes;

	H2B::INSTANCED_MESH instancedMesh = H2B::INSTANCED_MESH();
	bool exists = FileExists(path);
	bool isUnique = IsUniqueMesh(container, assetName);
	if (exists && isUnique && LoadH2B(path, instancedMesh))
	{
		instancedMesh.meshName = assetName;
		instancedMesh.numInstances = 1;
		instancedMesh.matrices.push_back(world);
		container[assetName] = instancedMesh;

	}
	else if (exists && !isUnique)
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
	else if (strcmp(buffer, "Directional") == 0)
	{
		information.row1.w = 0.0f;
	}
	H2B::LIGHT l = H2B::LIGHT(
		information.row1,
		information.row2,
		information.row3,
		information.row4
	);
	uniqueLights.push_back(l);
}

void Level::LoadCameraFromFile()
{
	char buffer[256] = {};
	input.getline(buffer, 256);
	GW::MATH::GMATRIXF information = ReadMatrixData();
	camera = information;
}

BOOL Level::IsUniqueMesh(const std::map<std::string, H2B::INSTANCED_MESH>& container, const std::string& assetName)
{
	auto containerIter = container.find(assetName);
	BOOL IsUnique = (containerIter != container.end()) ? FALSE : TRUE;
	return IsUnique;
}

GW::MATH::GAABBMMF Level::GenerateAABB(const std::vector<H2B::VERTEX>& verts)
{
	GW::MATH::GAABBMMF aabb;
	aabb.min = { FLT_MAX, FLT_MAX, FLT_MAX, 1 };
	aabb.max = { -FLT_MAX, -FLT_MAX, -FLT_MAX, 1 };

	for (const auto& vertex : verts)
	{
		aabb.min.x = min(aabb.min.x, vertex.pos.x);
		aabb.min.y = min(aabb.min.y, vertex.pos.y);
		aabb.min.z = min(aabb.min.z, vertex.pos.z);
		aabb.max.x = max(aabb.max.x, vertex.pos.x);
		aabb.max.y = max(aabb.max.y, vertex.pos.y);
		aabb.max.z = max(aabb.max.z, vertex.pos.z);
	}

	return aabb;
}
