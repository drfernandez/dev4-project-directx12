#pragma once

#include <commdlg.h>
// minimalistic code to draw a single triangle, this is not part of the API.
// required for compiling shaders on the fly, consider pre-compiling instead
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")
#include "d3dx12.h" // official helper file provided by microsoft
//#include "FSLogo.h"
#include "level.h"
#include <DDSTextureLoader.h>
#include <WICTextureLoader.h>


#define D3D12_SAFE_RELEASE(ptr) { if(ptr) { ptr->Release(); ptr = nullptr; } }	// releasing and setting to null (releases x2)
#define D3D12_COMPTR_SAFE_RELEASE(ptr) { if(ptr) { ptr = nullptr; } }
#define MAX_TEXTURES 1000

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

	UINT64											fenceValues;

	GW::INPUT::GInput								kbmProxy;
	GW::INPUT::GController							controllerProxy;
	GW::INPUT::GBufferedInput						bufferedInput;
	GW::CORE::GEventResponder						eventResponder;
	bool											dialogBoxOpen;

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
	UINT											cbvsrvuabIndex;

	Microsoft::WRL::ComPtr<ID3D12Resource>			constantBufferScene;
	CD3DX12_CPU_DESCRIPTOR_HANDLE					cbvSceneHandle;
	SCENE*											constantBufferSceneData;

	Microsoft::WRL::ComPtr<ID3D12Resource>			structuredBufferAttributesResource;
	CD3DX12_CPU_DESCRIPTOR_HANDLE					structuredBufferAttributeHandle;
	H2B::ATTRIBUTES*								structuredBufferAttributesData;

	Microsoft::WRL::ComPtr<ID3D12Resource>			structuredBufferInstanceResource;
	CD3DX12_CPU_DESCRIPTOR_HANDLE					structuredBufferInstanceHandle;
	GW::MATH::GMATRIXF*								structuredBufferInstanceData;

	Microsoft::WRL::ComPtr<ID3D12Resource>			structuredBufferLightResource;
	CD3DX12_CPU_DESCRIPTOR_HANDLE					structuredBufferLightHandle;
	H2B::LIGHT*										structuredBufferLightData;

	Level											currentLevel;

	UINT CalculateConstantBufferByteSize(UINT byteSize);
	std::string ShaderAsString(const CHAR* shaderFilePath);

	VOID UpdateCamera(FLOAT deltaTime);
	VOID ReleaseLevelResources();
	BOOL LoadLevelDataFromFile(const std::string& filename);
	bool OpenFileDialogBox(GW::SYSTEM::UNIVERSAL_WINDOW_HANDLE windowHandle, std::string& fileName);
	VOID WaitForGpu();

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
		FLOAT total_pitch = G_DEGREE_TO_RADIAN(65.0f) * (mouse_y_delta / static_cast<FLOAT>(screen_height)) + (controllerState[G_RY_AXIS] * thumb_speed);
		GW::MATH::GMATRIXF x_rotation = GW::MATH::GIdentityMatrixF;
		matrixProxy.RotateXLocalF(x_rotation, total_pitch, x_rotation);
		matrixProxy.MultiplyMatrixF(x_rotation, worldCamera, worldCamera);
	}
	if (mouse_moved || aim_left_right_changed)
	{
		FLOAT total_yaw = G_DEGREE_TO_RADIAN(65.0f) * aspect_ratio * (mouse_x_delta / static_cast<FLOAT>(screen_width)) + (controllerState[G_RX_AXIS] * thumb_speed);
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

VOID Renderer::ReleaseLevelResources()
{
	currentLevel.Clear();

	vertexView = { 0 };
	indexView = { 0 };

	D3D12_COMPTR_SAFE_RELEASE(vertexBuffer);
	D3D12_COMPTR_SAFE_RELEASE(indexBuffer);
	D3D12_COMPTR_SAFE_RELEASE(constantBufferScene);
	D3D12_COMPTR_SAFE_RELEASE(structuredBufferAttributesResource);
	D3D12_COMPTR_SAFE_RELEASE(structuredBufferInstanceResource);
	D3D12_COMPTR_SAFE_RELEASE(structuredBufferLightResource);

	constantBufferSceneData = nullptr;
	structuredBufferAttributesData = nullptr;
	structuredBufferInstanceData = nullptr;
	structuredBufferLightData = nullptr;
}

BOOL Renderer::LoadLevelDataFromFile(const std::string& filename)
{
	ID3D12Device* device = nullptr;
	d3d.GetDevice((void**)&device);

	if (!currentLevel.LoadLevel(filename))
	{
		return FALSE;
	}

	worldCamera = currentLevel.camera;


	HRESULT hr = E_NOTIMPL;

	{
		////////////////////////////////////////////////////////////////////////////////////////////////////
		// vertex buffer creation
		UINT vertexBufferSize = sizeof(H2B::VERTEX) * currentLevel.vertex_count;
		hr = device->CreateCommittedResource( // using UPLOAD heap for simplicity
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // DEFAULT recommend  
			D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(vertexBuffer.ReleaseAndGetAddressOf()));

		UINT8* transferMemoryLocation = nullptr;
		hr = vertexBuffer->Map(0, &CD3DX12_RANGE(0, 0), reinterpret_cast<void**>(&transferMemoryLocation));
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
		hr = device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(indexBuffer.ReleaseAndGetAddressOf()));

		UINT8* transferMemoryLocation = nullptr;
		hr = indexBuffer->Map(0, &CD3DX12_RANGE(0, 0), reinterpret_cast<void**>(&transferMemoryLocation));
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
		cbvsrvuavHeapDesc.NumDescriptors = 4 + MAX_TEXTURES;		// X number of descriptors to create from cbv_srv_uav heap
		cbvsrvuavHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvsrvuavHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		hr = device->CreateDescriptorHeap(&cbvsrvuavHeapDesc, IID_PPV_ARGS(cbvsrvuavHeap.ReleaseAndGetAddressOf()));

		

		cbvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		////////////////////////////////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////////////////////////////////////
		UINT constantBufferSize = CalculateConstantBufferByteSize(sizeof(SCENE));
		CD3DX12_HEAP_PROPERTIES prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC resource_desc = CD3DX12_RESOURCE_DESC::Buffer(constantBufferSize);
		// commited resource for the constant buffer
		hr = device->CreateCommittedResource(
			&prop,
			D3D12_HEAP_FLAG_NONE,
			&resource_desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(constantBufferScene.ReleaseAndGetAddressOf()));

		cbvSceneHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(cbvsrvuavHeap->GetCPUDescriptorHandleForHeapStart(), cbvsrvuabIndex, cbvDescriptorSize);
		cbvsrvuabIndex++;

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = constantBufferScene->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = constantBufferSize;    // CB size is required to be 256-byte aligned.
		// create CBV
		device->CreateConstantBufferView(&cbvDesc, cbvSceneHandle);
		////////////////////////////////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////////////////////////////////////
		// materials
		const UINT numAttributes = currentLevel.mm->material_count;
		if (numAttributes > 0)
		{
			// commited resource for the structured buffer
			resource_desc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(H2B::ATTRIBUTES) * numAttributes);
			hr = device->CreateCommittedResource(
				&prop,
				D3D12_HEAP_FLAG_NONE,
				&resource_desc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(structuredBufferAttributesResource.ReleaseAndGetAddressOf()));

			structuredBufferAttributeHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(cbvsrvuavHeap->GetCPUDescriptorHandleForHeapStart(), cbvsrvuabIndex, cbvDescriptorSize);
			cbvsrvuabIndex++;

			// srv description
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.NumElements = numAttributes;
			srvDesc.Buffer.StructureByteStride = sizeof(H2B::ATTRIBUTES);
			// create SRV
			device->CreateShaderResourceView(structuredBufferAttributesResource.Get(), &srvDesc, structuredBufferAttributeHandle);
		}
		////////////////////////////////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////////////////////////////////////
		// instance data for matrices
		const UINT numInstances = currentLevel.instanceData.size();
		if (numInstances > 0)
		{
			// commited resource for the structured buffer
			resource_desc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(GW::MATH::GMATRIXF) * numInstances);
			hr = device->CreateCommittedResource(
				&prop,
				D3D12_HEAP_FLAG_NONE,
				&resource_desc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(structuredBufferInstanceResource.ReleaseAndGetAddressOf()));

			structuredBufferInstanceHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(cbvsrvuavHeap->GetCPUDescriptorHandleForHeapStart(), cbvsrvuabIndex, cbvDescriptorSize);
			cbvsrvuabIndex++;

			// srv description
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.NumElements = numInstances;
			srvDesc.Buffer.StructureByteStride = sizeof(GW::MATH::GMATRIXF);
			// create SRV
			device->CreateShaderResourceView(structuredBufferInstanceResource.Get(), &srvDesc, structuredBufferInstanceHandle);
		}
		////////////////////////////////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////////////////////////////////////
		// lighting information
		const UINT numLights = currentLevel.uniqueLights.size();
		if (numLights > 0)
		{
			// commited resource for the structured buffer
			resource_desc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(H2B::LIGHT) * numLights);
			hr = device->CreateCommittedResource(
				&prop,
				D3D12_HEAP_FLAG_NONE,
				&resource_desc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(structuredBufferLightResource.ReleaseAndGetAddressOf()));

			structuredBufferLightHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(cbvsrvuavHeap->GetCPUDescriptorHandleForHeapStart(), cbvsrvuabIndex, cbvDescriptorSize);
			cbvsrvuabIndex++;

			// srv description
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.NumElements = numLights;
			srvDesc.Buffer.StructureByteStride = sizeof(H2B::LIGHT);
			// create SRV
			device->CreateShaderResourceView(structuredBufferLightResource.Get(), &srvDesc, structuredBufferLightHandle);
		}
		////////////////////////////////////////////////////////////////////////////////////////////////////
		

		////////////////////////////////////////////////////////////////////////////////////////////////////
		// Texture Loading 
		{
			std::wstring filepathDDS = L"../textures/uvmapping.dds";
			//std::wstring filepathWIC = L"../textures/uvmapping.jpg";

			std::unique_ptr<uint8_t[]> ddsData;
			std::vector<D3D12_SUBRESOURCE_DATA> subresources;
			Microsoft::WRL::ComPtr<ID3D12Resource> textureResourceDDS;
			DirectX::DDS_ALPHA_MODE alphaMode;
			bool isCubeMap = false;
			hr = DirectX::LoadDDSTextureFromFile(device, filepathDDS.c_str(), textureResourceDDS.ReleaseAndGetAddressOf(),
				ddsData, subresources, 0Ui64, &alphaMode, &isCubeMap);

			D3D12_RESOURCE_DESC resourceDesc = textureResourceDDS->GetDesc();
			D3D12_SHADER_RESOURCE_VIEW_DESC ddsSrvDesc = D3D12_SHADER_RESOURCE_VIEW_DESC();
			ddsSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			ddsSrvDesc.Format = resourceDesc.Format;
			ddsSrvDesc.ViewDimension = (isCubeMap) ? D3D12_SRV_DIMENSION_TEXTURECUBE : D3D12_SRV_DIMENSION_TEXTURE2D;
			ddsSrvDesc.Texture2D.MipLevels = resourceDesc.MipLevels;

			CD3DX12_CPU_DESCRIPTOR_HANDLE descHandle(cbvsrvuavHeap->GetCPUDescriptorHandleForHeapStart(), cbvsrvuabIndex, cbvDescriptorSize);
			cbvsrvuabIndex++;		

			device->CreateShaderResourceView(textureResourceDDS.Get(), &ddsSrvDesc, descHandle);
			

			//std::unique_ptr<uint8_t[]> wicData;
			//D3D12_SUBRESOURCE_DATA subresource;
			//Microsoft::WRL::ComPtr<ID3D12Resource> textureResourceWIC;
			//hr = DirectX::LoadWICTextureFromFile(device, filepathWIC.c_str(), textureResourceWIC.ReleaseAndGetAddressOf(), wicData, subresource);
		}
		////////////////////////////////////////////////////////////////////////////////////////////////////
		

		////////////////////////////////////////////////////////////////////////////////////////////////////
		// Map all the necessary resources to a GPU address in order to copy data from the CPU
		if (constantBufferScene)
		{
			hr = constantBufferScene->Map(0, &CD3DX12_RANGE(0, 0), reinterpret_cast<void**>(&constantBufferSceneData));
		}

		if (structuredBufferAttributesResource)
		{
			hr = structuredBufferAttributesResource->Map(0, &CD3DX12_RANGE(0, 0), reinterpret_cast<void**>(&structuredBufferAttributesData));
			if (SUCCEEDED(hr))
			{
				memcpy(structuredBufferAttributesData, currentLevel.mm->materials.data(), sizeof(H2B::ATTRIBUTES) * numAttributes);
				structuredBufferAttributesResource->Unmap(0, nullptr);
			}
		}

		if (structuredBufferInstanceResource)
		{
			hr = structuredBufferInstanceResource->Map(0, &CD3DX12_RANGE(0, 0), reinterpret_cast<void**>(&structuredBufferInstanceData));
			if (SUCCEEDED(hr))
			{
				memcpy(structuredBufferInstanceData, currentLevel.instanceData.data(), sizeof(GW::MATH::GMATRIXF) * numInstances);
				structuredBufferInstanceResource->Unmap(0, nullptr);
			}
		}

		if (structuredBufferLightResource)
		{
			hr = structuredBufferLightResource->Map(0, &CD3DX12_RANGE(0, 0), reinterpret_cast<void**>(&structuredBufferLightData));
			if (SUCCEEDED(hr))
			{
				memcpy(structuredBufferLightData, currentLevel.uniqueLights.data(), sizeof(H2B::LIGHT) * numLights);
				structuredBufferLightResource->Unmap(0, nullptr);
			}
		}
		////////////////////////////////////////////////////////////////////////////////////////////////////
	}

	device->Release();

	return TRUE;
}


bool Renderer::OpenFileDialogBox(GW::SYSTEM::UNIVERSAL_WINDOW_HANDLE windowHandle, std::string& fileName)
{
	bool result = false;
	OPENFILENAME ofn = { 0 };       // common dialog box structure
	WCHAR szFile[260];       // buffer for file name 
	HWND hwnd = (HWND)windowHandle.window;              // owner window

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = szFile;
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = L"Text\0*.TXT\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// Display the Open dialog box.
	if (GetOpenFileNameW(&ofn) == TRUE)
	{
		std::wstring ws(ofn.lpstrFile);
		fileName = std::string(ws.begin(), ws.end());
		result = true;
	}
	return result;
}

// Wait for pending GPU work to complete.
VOID Renderer::WaitForGpu()
{
	ID3D12CommandQueue* commandQueue;
	ID3D12Fence*	fence;
	ULONG ref = 0;

	d3d.GetCommandQueue((void**)&commandQueue);
	d3d.GetFence((void**)&fence);
	
	// Schedule a Signal command in the queue.
	commandQueue->Signal(fence, fenceValues);

	// Wait until the fence has been processed.
	fence->SetEventOnCompletion(fenceValues, nullptr);

	// Increment the fence value for the current frame.
	fenceValues++;
	ref = commandQueue->Release();
	ref = fence->Release();
}

Renderer::Renderer(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GDirectX12Surface _d3d)
{
	dialogBoxOpen = false;
	fenceValues = 0;
	cbvsrvuabIndex = 1;

	win = _win;
	d3d = _d3d;
	ID3D12Device* creator;
	d3d.GetDevice((void**)&creator);

	matrixProxy.Create();
	kbmProxy.Create(win);
	controllerProxy.Create();
	bufferedInput.Create(win);

	GW::SYSTEM::UNIVERSAL_WINDOW_HANDLE uwh;
	+win.GetWindowHandle(uwh);

	ReleaseLevelResources();

	+eventResponder.Create([=](const GW::GEvent& e)
		{
			bool IsFocusWindow = GetFocus() == (HWND&)uwh;
			if (!IsFocusWindow)
			{
				return;
			}
			GW::INPUT::GBufferedInput::Events q;
			GW::INPUT::GBufferedInput::EVENT_DATA qd;
			bool bReadIsGood = +e.Read(q, qd);
			bool bBufferedInputKey1 = (q == GW::INPUT::GBufferedInput::Events::KEYRELEASED && qd.data == G_KEY_1);
			if (bReadIsGood && bBufferedInputKey1 && !dialogBoxOpen)
			{
				dialogBoxOpen = true;
				std::string levelName = std::string();
				if (OpenFileDialogBox(uwh, levelName))
				{
					WaitForGpu();
					ReleaseLevelResources();
					LoadLevelDataFromFile(levelName);
				}
				dialogBoxOpen = false;
			}
		});
	+bufferedInput.Register(eventResponder);


	HRESULT hr = E_NOTIMPL;

	{
		identityMatrix = GW::MATH::GIdentityMatrixF;
	}

	{
		// view and projection creation

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

	CD3DX12_DESCRIPTOR_RANGE ranges[1] = {};
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, MAX_TEXTURES, 3, 0);

	CD3DX12_ROOT_PARAMETER rootParameters[6] = {};
	rootParameters[0].InitAsConstants(2, 0, 0);			// num32bitConstants, register, space  (mesh_id)	b0
	rootParameters[1].InitAsConstantBufferView(1, 0);	// register, space	(view,projection)				b1
	rootParameters[2].InitAsShaderResourceView(0, 0);	// register, space	(OBJ_ATTRIBUTES)				t0
	rootParameters[3].InitAsShaderResourceView(1, 0);	// register, space	(instance matrix data)			t1
	rootParameters[4].InitAsShaderResourceView(2, 0);	// register, space	(light data)					t2
	rootParameters[5].InitAsDescriptorTable(1, &ranges[0]);

	// static samplers
	CD3DX12_STATIC_SAMPLER_DESC sampler(
		0, 
		D3D12_FILTER_ANISOTROPIC,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		0.0f, 16U, 
		D3D12_COMPARISON_FUNC_LESS_EQUAL, 
		D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
		0.0f, 3.402823466e+38F, 
		D3D12_SHADER_VISIBILITY_PIXEL, 
		0);

	// create root signature
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Init(
		ARRAYSIZE(rootParameters), rootParameters, // number of params, root parameters
		1, &sampler, // number of samplers, static samplers
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	Microsoft::WRL::ComPtr<ID3DBlob> signature;
	hr = D3D12SerializeRootSignature(&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1, &signature, &errors);
	if(FAILED(hr))
	{
		std::cout << (char*)errors->GetBufferPointer() << std::endl;
		abort();
	}

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
	hr = creator->CreateGraphicsPipelineState(&psDesc, IID_PPV_ARGS(&pipeline));
	if(FAILED(hr))
	{
		abort();
	}
	// free temporary handle
	creator->Release();
}

Renderer::~Renderer()
{
	// ComPtr will auto release so nothing to do here 
	if (constantBufferScene)
	{
		constantBufferScene->Unmap(0, nullptr);
	}
}

VOID Renderer::Render()
{
	if (!vertexView.BufferLocation || !indexView.BufferLocation)
		return;

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

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> ppHeaps[] = { cbvsrvuavHeap.Get() };
	cmd->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps->GetAddressOf());

	// now we can draw
	cmd->IASetVertexBuffers(0, 1, &vertexView);
	cmd->IASetIndexBuffer(&indexView);
	cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	CD3DX12_GPU_DESCRIPTOR_HANDLE srvHandle(cbvsrvuavHeap->GetGPUDescriptorHandleForHeapStart(), 0, cbvDescriptorSize);
	cmd->SetGraphicsRootDescriptorTable(5, srvHandle);

	if (constantBufferScene)
	{
		D3D12_GPU_VIRTUAL_ADDRESS cbvHandle = constantBufferScene->GetGPUVirtualAddress() + 0U * (unsigned long long)CalculateConstantBufferByteSize(sizeof(SCENE));
		cmd->SetGraphicsRootConstantBufferView(1, cbvHandle);
	}
	if (structuredBufferAttributesResource)
	{
		D3D12_GPU_VIRTUAL_ADDRESS srvAttributesHandle = structuredBufferAttributesResource->GetGPUVirtualAddress() + 0U * (sizeof(H2B::ATTRIBUTES));
		cmd->SetGraphicsRootShaderResourceView(2, srvAttributesHandle);
	}
	if (structuredBufferInstanceResource)
	{
		D3D12_GPU_VIRTUAL_ADDRESS srvInstanceHandle = structuredBufferInstanceResource->GetGPUVirtualAddress() + 0U * sizeof(GW::MATH::GMATRIXF);
		cmd->SetGraphicsRootShaderResourceView(3, srvInstanceHandle);
	}
	if (structuredBufferLightResource)
	{
		D3D12_GPU_VIRTUAL_ADDRESS srvLightHandle = structuredBufferLightResource->GetGPUVirtualAddress() + 0U * sizeof(H2B::LIGHT);
		cmd->SetGraphicsRootShaderResourceView(4, srvLightHandle);
	}

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
	GW::SYSTEM::UNIVERSAL_WINDOW_HANDLE wndHandle;
	win.GetWindowHandle(wndHandle);
	BOOL IsFocusWindow = GetFocus() == (HWND&)wndHandle;

	if (IsFocusWindow)
	{
		UpdateCamera(deltaTime);
	}


	GW::MATH::GVECTORF campos = worldCamera.row4;
	campos.w = currentLevel.uniqueLights.size();
	SCENE scene =
	{
		viewMatrix,
		projectionMatrix,
		campos
	};

	if (constantBufferSceneData)
	{
		memcpy(constantBufferSceneData, &scene, sizeof(SCENE));
	}
}