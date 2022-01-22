#pragma once

// minimalistic code to draw a single triangle, this is not part of the API.
// required for compiling shaders on the fly, consider pre-compiling instead
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")
#include "d3dx12.h" // official helper file provided by microsoft
//#include "FSLogo.h"
#include "level.h"

#define D3D12_SAFE_RELEASE(ptr) { if(ptr) { ptr->Release(); ptr = nullptr; } }

struct SCENE
{
	GW::MATH::GMATRIXF view;
	GW::MATH::GMATRIXF projection;
	GW::MATH::GVECTORF cameraPosition;
};

// Creation, Rendering & Cleanup
class Renderer
{
private:
	// proxy handles
	GW::SYSTEM::GWindow								win;
	GW::GRAPHICS::GDirectX12Surface					d3d;

	GW::INPUT::GInput								kbmProxy;
	GW::INPUT::GController							controllerProxy;
	GW::INPUT::GBufferedInput						bufferedInput;
	GW::CORE::GEventResponder						eventResponder;
	BOOL											dialogBoxOpen;

	GW::MATH::GMatrix								matrixProxy;
	GW::MATH::GMATRIXF								identityMatrix;
	GW::MATH::GMATRIXF								viewMatrix;
	GW::MATH::GMATRIXF								projectionMatrix;
	GW::MATH::GMATRIXF								worldCamera;

	D3D12_VERTEX_BUFFER_VIEW						vertexView;
	D3D12_INDEX_BUFFER_VIEW							indexView;
	Microsoft::WRL::ComPtr<ID3D12Resource>			vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource>			indexBuffer;

	Microsoft::WRL::ComPtr<ID3D12RootSignature>		rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState>		pipeline;

	UINT											cbvDescriptorSize;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	cbvsrvuavHeap;

	Microsoft::WRL::ComPtr<ID3D12Resource>			constantBufferScene;
	CD3DX12_CPU_DESCRIPTOR_HANDLE					cbvSceneHandle;
	SCENE*											constantBufferSceneData;

	Microsoft::WRL::ComPtr<ID3D12Resource>			structuredBufferAttributesResource;
	CD3DX12_CPU_DESCRIPTOR_HANDLE					structuredBufferAttributeHandle;
	H2B::ATTRIBUTES*								structuredBufferAttributesData;
	std::vector<H2B::ATTRIBUTES>					attributesData;

	Microsoft::WRL::ComPtr<ID3D12Resource>			structuredBufferInstanceResource;
	CD3DX12_CPU_DESCRIPTOR_HANDLE					structuredBufferInstanceHandle;
	GW::MATH::GMATRIXF*								structuredBufferInstanceData;
	std::vector<GW::MATH::GMATRIXF>					instanceData;

	Microsoft::WRL::ComPtr<ID3D12Resource>			structuredBufferLightResource;
	CD3DX12_CPU_DESCRIPTOR_HANDLE					structuredBufferLightHandle;
	H2B::LIGHT*										structuredBufferLightData;
	std::vector<H2B::LIGHT>							lightData;

	Level											currentLevel;

	UINT CalculateConstantBufferByteSize(UINT byteSize);
	std::string ShaderAsString(const CHAR* shaderFilePath);
	VOID UpdateCamera(FLOAT deltaTime);
	VOID ReleaseLevelData();
	BOOL LoadLevelDataFromFile(ID3D12Device* creator, const std::string& filename);

public:
	Renderer(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GDirectX12Surface _d3d);
	VOID Render();
	~Renderer();
	VOID Update(FLOAT deltaTime);

};


UINT Renderer::CalculateConstantBufferByteSize(UINT byteSize)
{
	return (byteSize + (D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1)) & ~(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1);
};

std::string Renderer::ShaderAsString(const CHAR* shaderFilePath)
{
	std::string output;
	unsigned int stringLength = 0;
	GW::SYSTEM::GFile file; file.Create();
	file.GetFileSize(shaderFilePath, stringLength);
	if (stringLength && +file.OpenBinaryRead(shaderFilePath))
	{
		output.resize(stringLength);
		file.Read(&output[0], stringLength);
	}
	return output;
}

VOID Renderer::UpdateCamera(FLOAT deltaTime)
{
	std::vector<FLOAT> kbmState(256);
	std::vector<FLOAT> controllerState(256);
	const FLOAT camera_speed = 0.5f;
	FLOAT per_frame_speed = camera_speed * deltaTime;
	FLOAT thumb_speed = G_PI_F * deltaTime * 0.05f;
	FLOAT aspect_ratio = 0.0f;
	UINT screen_width = 0;
	UINT screen_height = 0;
	FLOAT mouse_x_delta = 0.0f;
	FLOAT mouse_y_delta = 0.0f;

	win.GetWidth(screen_width);
	win.GetHeight(screen_height);
	d3d.GetAspectRatio(aspect_ratio);

	for (UINT i = 0; i < 256; i++)
	{
		kbmProxy.GetState(i, kbmState[i]);
		controllerProxy.GetState(0, i, controllerState[i]);
	}

	bool move_up_changed = kbmState[G_KEY_SPACE] || controllerState[G_RIGHT_TRIGGER_AXIS];
	bool move_down_changed = kbmState[G_KEY_LEFTSHIFT] || controllerState[G_LEFT_TRIGGER_AXIS];
	bool move_forward_changed = kbmState[G_KEY_W] || controllerState[G_LY_AXIS];
	bool move_backward_changed = kbmState[G_KEY_S] || controllerState[G_LY_AXIS];
	bool move_left_changed = kbmState[G_KEY_A] || controllerState[G_LX_AXIS];
	bool move_right_changed = kbmState[G_KEY_D] || controllerState[G_LX_AXIS];
	bool mouse_r_button_pressed = kbmState[G_BUTTON_RIGHT];
	bool aim_up_down_changed = controllerState[G_RY_AXIS];
	bool aim_left_right_changed = controllerState[G_RX_AXIS];
	GW::GReturn result = kbmProxy.GetMouseDelta(mouse_x_delta, mouse_y_delta);
	bool mouse_moved = G_PASS(result) && result != GW::GReturn::REDUNDANT && mouse_r_button_pressed;

	if (!mouse_moved)
	{
		mouse_x_delta = mouse_y_delta = 0.0f;
	}

	if (move_up_changed || move_down_changed)
	{
		FLOAT total_y_change = kbmState[G_KEY_SPACE] - kbmState[G_KEY_LEFTSHIFT] + controllerState[G_RIGHT_TRIGGER_AXIS] - controllerState[G_LEFT_TRIGGER_AXIS];
		worldCamera.row4.y += total_y_change * camera_speed * deltaTime;
	}
	if (move_forward_changed || move_backward_changed || move_left_changed || move_right_changed)
	{
		FLOAT total_x_change = kbmState[G_KEY_D] - kbmState[G_KEY_A] + controllerState[G_LX_AXIS];
		FLOAT total_z_change = kbmState[G_KEY_W] - kbmState[G_KEY_S] + controllerState[G_LY_AXIS];
		GW::MATH::GVECTORF translation = { total_x_change * per_frame_speed, 0.0f, total_z_change * per_frame_speed, 1.0f };
		matrixProxy.TranslateLocalF(worldCamera, translation, worldCamera);
	}
	if (mouse_moved || aim_up_down_changed)
	{
		FLOAT total_pitch = G_DEGREE_TO_RADIAN(65.0f) * mouse_y_delta / static_cast<FLOAT>(screen_height) + controllerState[G_RY_AXIS] * thumb_speed;
		GW::MATH::GMATRIXF x_rotation = GW::MATH::GIdentityMatrixF;
		matrixProxy.RotateXLocalF(x_rotation, total_pitch, x_rotation);
		matrixProxy.MultiplyMatrixF(x_rotation, worldCamera, worldCamera);
	}
	if (mouse_moved || aim_left_right_changed)
	{
		FLOAT total_yaw = G_DEGREE_TO_RADIAN(65.0f) * aspect_ratio * mouse_x_delta / static_cast<FLOAT>(screen_width) + controllerState[G_RX_AXIS] * thumb_speed;
		GW::MATH::GMATRIXF y_rotation = GW::MATH::GIdentityMatrixF;
		matrixProxy.RotateYLocalF(y_rotation, total_yaw, y_rotation);
		GW::MATH::GVECTORF position = worldCamera.row4;
		matrixProxy.MultiplyMatrixF(worldCamera, y_rotation, worldCamera);
		worldCamera.row4 = position;
	}


	// end of camera update
	matrixProxy.InverseF(worldCamera, viewMatrix);
	matrixProxy.ProjectionDirectXLHF(G2D_DEGREE_TO_RADIAN(65),
		aspect_ratio, 0.1f, 1000.0f,
		projectionMatrix);
}

VOID Renderer::ReleaseLevelData()
{
	currentLevel.Clear();
	currentLevel.mm->Clear();

	vertexView = { 0 };
	indexView = { 0 };
	D3D12_SAFE_RELEASE(vertexBuffer);
	D3D12_SAFE_RELEASE(indexBuffer);
	D3D12_SAFE_RELEASE(constantBufferScene);
	D3D12_SAFE_RELEASE(structuredBufferAttributesResource);
	D3D12_SAFE_RELEASE(structuredBufferInstanceResource);
	D3D12_SAFE_RELEASE(structuredBufferLightResource);
	attributesData.clear();
	instanceData.clear();
	lightData.clear();

}

BOOL Renderer::LoadLevelDataFromFile(ID3D12Device* creator, const std::string& filename)
{
	ReleaseLevelData();
	int d = 0;

	BOOL loaded = currentLevel.LoadLevel(filename);

	for (const auto& m : currentLevel.uniqueMeshes)
	{
		for (const auto& matrix : m.second.matrices)
		{
			instanceData.push_back(matrix);
		}
	}

	MaterialManager* mm = currentLevel.mm;
	for (UINT i = 0; i < mm->material_count; i++)
	{
		H2B::MATERIAL2 mat = H2B::MATERIAL2(mm->GetMaterial(i));
		attributesData.push_back(mat.attrib);
	}

	HRESULT hr = E_NOTIMPL;

	{
		////////////////////////////////////////////////////////////////////////////////////////////////////
		// vertex buffer creation
		UINT vertexBufferSize = sizeof(H2B::VERTEX) * currentLevel.vertex_count;
		creator->CreateCommittedResource( // using UPLOAD heap for simplicity
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // DEFAULT recommend  
			D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexBuffer));

		UINT8* transferMemoryLocation = nullptr;
		vertexBuffer->Map(0, &CD3DX12_RANGE(0, 0),
			reinterpret_cast<void**>(&transferMemoryLocation));
		memcpy(transferMemoryLocation, currentLevel.vertices.data(), vertexBufferSize);
		vertexBuffer->Unmap(0, nullptr);
		// Create a vertex View to send to a Draw() call.
		vertexView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
		vertexView.StrideInBytes = sizeof(H2B::VERTEX);
		vertexView.SizeInBytes = vertexBufferSize;
		////////////////////////////////////////////////////////////////////////////////////////////////////
	}
	{
		////////////////////////////////////////////////////////////////////////////////////////////////////
		// index buffer creation
		UINT indexBufferSize = sizeof(UINT) * currentLevel.index_count;
		creator->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&indexBuffer));

		UINT8* transferMemoryLocation = nullptr;
		indexBuffer->Map(0, &CD3DX12_RANGE(0, 0),
			reinterpret_cast<void**>(&transferMemoryLocation));
		memcpy(transferMemoryLocation, currentLevel.indices.data(), indexBufferSize);
		indexBuffer->Unmap(0, nullptr);
		indexView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
		indexView.Format = DXGI_FORMAT_R32_UINT;
		indexView.SizeInBytes = indexBufferSize;
		////////////////////////////////////////////////////////////////////////////////////////////////////
	}
	// constant buffer (CBV) / structured buffer (SRV) creation
	{
		////////////////////////////////////////////////////////////////////////////////////////////////////
		// constant buffer heap creation
		D3D12_DESCRIPTOR_HEAP_DESC cbvsrvuavHeapDesc = {};
		cbvsrvuavHeapDesc.NumDescriptors = 4;		// X number of descriptors to create from cbv_srv_uav heap
		cbvsrvuavHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvsrvuavHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		creator->CreateDescriptorHeap(&cbvsrvuavHeapDesc, IID_PPV_ARGS(cbvsrvuavHeap.ReleaseAndGetAddressOf()));

		cbvDescriptorSize = creator->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		////////////////////////////////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////////////////////////////////////
		UINT constantBufferSize = CalculateConstantBufferByteSize(sizeof(SCENE));
		CD3DX12_HEAP_PROPERTIES prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC resource_desc = CD3DX12_RESOURCE_DESC::Buffer(constantBufferSize);
		// commited resource for the constant buffer
		hr = creator->CreateCommittedResource(
			&prop,
			D3D12_HEAP_FLAG_NONE,
			&resource_desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&constantBufferScene));

		cbvSceneHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(cbvsrvuavHeap->GetCPUDescriptorHandleForHeapStart(), 0, cbvDescriptorSize);

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = constantBufferScene->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = constantBufferSize;    // CB size is required to be 256-byte aligned.
		// create CBV
		creator->CreateConstantBufferView(&cbvDesc, cbvSceneHandle);
		////////////////////////////////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////////////////////////////////////
		// materials
		const UINT numAttributes = currentLevel.mm->material_count;
		// commited resource for the structured buffer
		resource_desc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(H2B::ATTRIBUTES) * numAttributes);
		hr = creator->CreateCommittedResource(
			&prop,
			D3D12_HEAP_FLAG_NONE,
			&resource_desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&structuredBufferAttributesResource));

		structuredBufferAttributeHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(cbvsrvuavHeap->GetCPUDescriptorHandleForHeapStart(), 1, cbvDescriptorSize);

		// srv description
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = numAttributes;
		srvDesc.Buffer.StructureByteStride = sizeof(H2B::ATTRIBUTES);
		// create SRV
		creator->CreateShaderResourceView(structuredBufferAttributesResource.Get(), &srvDesc, structuredBufferAttributeHandle);
		////////////////////////////////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////////////////////////////////////
		// instance data for matrices
		const UINT numInstances = instanceData.size();
		// commited resource for the structured buffer
		resource_desc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(GW::MATH::GMATRIXF) * numInstances);
		hr = creator->CreateCommittedResource(
			&prop,
			D3D12_HEAP_FLAG_NONE,
			&resource_desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&structuredBufferInstanceResource));

		structuredBufferInstanceHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(cbvsrvuavHeap->GetCPUDescriptorHandleForHeapStart(), 2, cbvDescriptorSize);

		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = numInstances;
		srvDesc.Buffer.StructureByteStride = sizeof(GW::MATH::GMATRIXF);
		// create SRV
		creator->CreateShaderResourceView(structuredBufferInstanceResource.Get(), &srvDesc, structuredBufferInstanceHandle);
		////////////////////////////////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////////////////////////////////////
		// lighting information
		const UINT numLights = currentLevel.uniqueLights.size();
		// commited resource for the structured buffer
		resource_desc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(H2B::LIGHT) * numLights);
		hr = creator->CreateCommittedResource(
			&prop,
			D3D12_HEAP_FLAG_NONE,
			&resource_desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&structuredBufferLightResource));

		structuredBufferLightHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(cbvsrvuavHeap->GetCPUDescriptorHandleForHeapStart(), 3, cbvDescriptorSize);

		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = numLights;
		srvDesc.Buffer.StructureByteStride = sizeof(H2B::LIGHT);
		// create SRV
		creator->CreateShaderResourceView(structuredBufferLightResource.Get(), &srvDesc, structuredBufferLightHandle);
		////////////////////////////////////////////////////////////////////////////////////////////////////


		CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
		hr = constantBufferScene->Map(0, &readRange, reinterpret_cast<void**>(&constantBufferSceneData));

		hr = structuredBufferAttributesResource->Map(0, &readRange, reinterpret_cast<void**>(&structuredBufferAttributesData));
		memcpy(structuredBufferAttributesData, attributesData.data(), sizeof(H2B::ATTRIBUTES) * numAttributes);
		structuredBufferAttributesResource->Unmap(0, nullptr);

		hr = structuredBufferInstanceResource->Map(0, &readRange, reinterpret_cast<void**>(&structuredBufferInstanceData));
		memcpy(structuredBufferInstanceData, instanceData.data(), sizeof(GW::MATH::GMATRIXF) * numInstances);
		structuredBufferInstanceResource->Unmap(0, nullptr);

		hr = structuredBufferLightResource->Map(0, &readRange, reinterpret_cast<void**>(&structuredBufferLightData));
		memcpy(structuredBufferLightData, currentLevel.uniqueLights.data(), sizeof(H2B::LIGHT) * numLights);
		structuredBufferLightResource->Unmap(0, nullptr);
	}

	return loaded;
}

Renderer::Renderer(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GDirectX12Surface _d3d)
{
	win = _win;
	d3d = _d3d;
	ID3D12Device* creator;
	d3d.GetDevice((void**)&creator);

	matrixProxy.Create();
	kbmProxy.Create(_win);
	controllerProxy.Create();

	HRESULT hr = E_NOTIMPL;

	{
		identityMatrix = GW::MATH::GIdentityMatrixF;
		std::string levelName = "../levels/Modular Dungeon 3.txt";

		if (LoadLevelDataFromFile(creator, levelName))
		{
			int d = 0;
		}

	}

	{
		// view and projection creation
		worldCamera = currentLevel.camera;

		float fov = G_DEGREE_TO_RADIAN(65);
		float zn = 0.1f;
		float zf = 100.0f;
		float aspect = 0.0f;
		d3d.GetAspectRatio(aspect);
		matrixProxy.ProjectionDirectXLHF(fov, aspect, zn, zf, projectionMatrix);
	}



	UINT compilerFlags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES;
#if _DEBUG
	compilerFlags |= D3DCOMPILE_DEBUG;
#endif
	std::string vertexShaderString = ShaderAsString("../shaders/vertexshader.hlsl");
	std::string pixelShaderString = ShaderAsString("../shaders/pixelshader.hlsl");

	// Create Vertex Shader
	Microsoft::WRL::ComPtr<ID3DBlob> vsBlob, errors;
	if (FAILED(D3DCompile(vertexShaderString.c_str(), vertexShaderString.length(),
		nullptr, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_1", compilerFlags, 0,
		vsBlob.GetAddressOf(), errors.GetAddressOf())))
	{
		std::cout << (char*)errors->GetBufferPointer() << std::endl;
		abort();
	}
	// Create Pixel Shader
	Microsoft::WRL::ComPtr<ID3DBlob> psBlob; errors.Reset();
	if (FAILED(D3DCompile(pixelShaderString.c_str(), pixelShaderString.length(),
		nullptr, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_1", compilerFlags, 0,
		psBlob.GetAddressOf(), errors.GetAddressOf())))
	{
		std::cout << (char*)errors->GetBufferPointer() << std::endl;
		abort();
	}
	// Create Input Layout
	D3D12_INPUT_ELEMENT_DESC format[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	//CD3DX12_DESCRIPTOR_RANGE ranges[1] = {};
	//ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

	CD3DX12_ROOT_PARAMETER rootParameters[5] = {};
	rootParameters[0].InitAsConstants(2, 0, 0);			// num32bitConstants, register, space  (mesh_id)	b0
	rootParameters[1].InitAsConstantBufferView(1, 0);	// register, space	(view,projection)				b1
	rootParameters[2].InitAsShaderResourceView(0, 0);	// register, space	(OBJ_ATTRIBUTES)				t0
	rootParameters[3].InitAsShaderResourceView(1, 0);	// register, space	(instance matrix data)			t1
	rootParameters[4].InitAsShaderResourceView(2, 0);	// register, space	(light data)					t2
	//rootParameters[4].InitAsDescriptorTable(ARRAYSIZE(ranges), ranges);


	// create root signature
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Init(
		ARRAYSIZE(rootParameters), rootParameters, // number of params, root parameters
		0, nullptr, // number of samplers, static samplers
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	Microsoft::WRL::ComPtr<ID3DBlob> signature;
	hr = D3D12SerializeRootSignature(&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1, &signature, &errors);

	creator->CreateRootSignature(0, signature->GetBufferPointer(),
		signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature));

	// create pipeline state
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psDesc = {};
	psDesc.InputLayout = { format, ARRAYSIZE(format) };
	psDesc.pRootSignature = rootSignature.Get();
	psDesc.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
	psDesc.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());
	psDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psDesc.SampleMask = UINT_MAX;
	psDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psDesc.NumRenderTargets = 1;
	psDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psDesc.SampleDesc.Count = 1;
	creator->CreateGraphicsPipelineState(&psDesc, IID_PPV_ARGS(&pipeline));
	// free temporary handle
	creator->Release();
}

Renderer::~Renderer()
{
	// ComPtr will auto release so nothing to do here 
	constantBufferScene->Unmap(0, nullptr);
}

VOID Renderer::Render()
{
	// grab the context & render target
	ID3D12GraphicsCommandList* cmd;
	D3D12_CPU_DESCRIPTOR_HANDLE rtv;
	D3D12_CPU_DESCRIPTOR_HANDLE dsv;
	d3d.GetCommandList((void**)&cmd);
	d3d.GetCurrentRenderTargetView((void**)&rtv);
	d3d.GetDepthStencilView((void**)&dsv);

	// setup the pipeline
	cmd->OMSetRenderTargets(1, &rtv, FALSE, &dsv);
	cmd->SetGraphicsRootSignature(rootSignature.Get());
	cmd->SetPipelineState(pipeline.Get());

	// now we can draw
	D3D12_GPU_VIRTUAL_ADDRESS cbvHandle = constantBufferScene->GetGPUVirtualAddress() + 0U * (unsigned long long)CalculateConstantBufferByteSize(sizeof(SCENE));
	D3D12_GPU_VIRTUAL_ADDRESS srvAttributesHandle = structuredBufferAttributesResource->GetGPUVirtualAddress() + 0U * (sizeof(H2B::ATTRIBUTES));
	D3D12_GPU_VIRTUAL_ADDRESS srvInstanceHandle = structuredBufferInstanceResource->GetGPUVirtualAddress() + 0U * sizeof(GW::MATH::GMATRIXF);
	D3D12_GPU_VIRTUAL_ADDRESS srvLightHandle = structuredBufferLightResource->GetGPUVirtualAddress() + 0U * sizeof(H2B::LIGHT);
	cmd->IASetVertexBuffers(0, 1, &vertexView);
	cmd->IASetIndexBuffer(&indexView);
	cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmd->SetGraphicsRootConstantBufferView(1, cbvHandle);
	cmd->SetGraphicsRootShaderResourceView(2, srvAttributesHandle);
	cmd->SetGraphicsRootShaderResourceView(3, srvInstanceHandle);
	cmd->SetGraphicsRootShaderResourceView(4, srvLightHandle);

	for (const auto& mesh : currentLevel.uniqueMeshes)	// mesh count
	{
		for (const auto& submesh : mesh.second.subMeshes)	// submesh count
		{
			UINT root32BitConstants[2] = { mesh.second.meshID, submesh.materialIndex };	// mesh_id, submesh_id, material_id?
			cmd->SetGraphicsRoot32BitConstants(0, ARRAYSIZE(root32BitConstants), root32BitConstants, 0);
			cmd->DrawIndexedInstanced(submesh.drawInfo.indexCount, mesh.second.numInstances, submesh.drawInfo.indexOffset, mesh.second.vertexOffset, 0);
		}
	}
	// release temp handles
	cmd->Release();
}

VOID Renderer::Update(FLOAT deltaTime)
{
	UpdateCamera(deltaTime);

	GW::MATH::GVECTORF campos = worldCamera.row4;
	campos.w = currentLevel.uniqueLights.size();
	SCENE scene =
	{
		viewMatrix,
		projectionMatrix,
		campos
	};

	memcpy(constantBufferSceneData, &scene, sizeof(SCENE));

}