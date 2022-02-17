#pragma once

// minimalistic code to draw a single triangle, this is not part of the API.
// required for compiling shaders on the fly, consider pre-compiling instead
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")
#include "d3dx12.h" // official helper file provided by microsoft
#include "level.h"
#include <DDSTextureLoader.h>
#include <locale> 
#include <codecvt>
#include <commdlg.h>
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_dx12.h"
#include "../imgui/imgui_impl_win32.h"


#define D3D12_SAFE_RELEASE(ptr) { if(ptr) { ptr->Release(); ptr = nullptr; } }	// releasing and setting to null (releases x2)
#define D3D12_COMPTR_SAFE_RELEASE(ptr) { if(ptr) { ptr = nullptr; } }
#define MAX_TEXTURE_COUNT 500
#define MAX_COLOR_TEXTURES MAX_TEXTURE_COUNT
#define MAX_NORMAL_TEXTURES MAX_TEXTURE_COUNT
#define MAX_SPECULAR_TEXTURES MAX_TEXTURE_COUNT

struct SCENE
{
	GW::MATH::GMATRIXF view;
	GW::MATH::GMATRIXF projection;
	GW::MATH::GVECTORF cameraPosition;
};

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


// Creation, Rendering & Cleanup
class Renderer
{
private:
	// proxy handles
	GW::SYSTEM::GWindow											win;
	GW::GRAPHICS::GDirectX12Surface								d3d;

	UINT64														fenceValues;

	GW::INPUT::GInput											kbmProxy;
	GW::INPUT::GController										controllerProxy;
	GW::INPUT::GBufferedInput									bufferedInput;
	GW::CORE::GEventResponder									eventResponder;
	bool														dialogBoxOpen;

	GW::MATH::GMatrix											matrixProxy;
	GW::MATH::GMATRIXF											identityMatrix;
	GW::MATH::GMATRIXF											viewMatrix;
	GW::MATH::GMATRIXF											projectionMatrix;
	GW::MATH::GMATRIXF											worldCamera;

	D3D12_VERTEX_BUFFER_VIEW									vertexView;
	D3D12_INDEX_BUFFER_VIEW										indexView;
	Microsoft::WRL::ComPtr<ID3D12Resource>						vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource>						indexBuffer;

	Microsoft::WRL::ComPtr<ID3D12RootSignature>					rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState>					pipeline;
	Microsoft::WRL::ComPtr<ID3D12PipelineState>					pipelineSkybox;

	UINT														cbvDescriptorSize;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>				cbvsrvuavHeap;

	Microsoft::WRL::ComPtr<ID3D12Resource>						constantBufferScene;
	CD3DX12_CPU_DESCRIPTOR_HANDLE								cbvSceneHandle;
	SCENE* constantBufferSceneData;

	Microsoft::WRL::ComPtr<ID3D12Resource>						structuredBufferAttributesResource;
	CD3DX12_CPU_DESCRIPTOR_HANDLE								structuredBufferAttributeHandle;
	H2B::ATTRIBUTES* structuredBufferAttributesData;

	Microsoft::WRL::ComPtr<ID3D12Resource>						structuredBufferInstanceResource;
	CD3DX12_CPU_DESCRIPTOR_HANDLE								structuredBufferInstanceHandle;
	GW::MATH::GMATRIXF* structuredBufferInstanceData;

	Microsoft::WRL::ComPtr<ID3D12Resource>						structuredBufferLightResource;
	CD3DX12_CPU_DESCRIPTOR_HANDLE								structuredBufferLightHandle;
	H2B::LIGHT* structuredBufferLightData;

	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>			textureResourceDiffuse;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>			textureResourceDiffuseUpload;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>			textureResourceNormal;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>			textureResourceNormalUpload;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>			textureResourceSpecular;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>			textureResourceSpecularUpload;
	UINT														textureBitMask;

	Microsoft::WRL::ComPtr<ID3D12Resource>						textureResourceDefault2D;
	Microsoft::WRL::ComPtr<ID3D12Resource>						textureResourceDefault2DUpload;
	Microsoft::WRL::ComPtr<ID3D12Resource>						textureResourceDefault3D;
	Microsoft::WRL::ComPtr<ID3D12Resource>						textureResourceDefault3DUpload;

	Level														currentLevel;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>				imguiSrvDescHeap;
	static LONG_PTR												gatewareWndProc;

	// used by Dear ImGui
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	VOID DisplayImguiMenu(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmd);

	UINT CalculateConstantBufferByteSize(UINT byteSize);
	std::string ShaderAsString(const CHAR* shaderFilePath);

	VOID UpdateCamera(FLOAT deltaTime);
	VOID ReleaseLevelResources();
	BOOL LoadLevelDataFromFile(const std::string& filename);
	bool OpenFileDialogBox(GW::SYSTEM::UNIVERSAL_WINDOW_HANDLE windowHandle, std::string& fileName);
	VOID WaitForGpu();
	HRESULT LoadVertexData(Microsoft::WRL::ComPtr<ID3D12Device> device,
		void* data, UINT strideInBytes, UINT bufferSizeInBytes,
		Microsoft::WRL::ComPtr<ID3D12Resource>& resource, D3D12_VERTEX_BUFFER_VIEW& vertexView);
	HRESULT LoadIndexData(Microsoft::WRL::ComPtr<ID3D12Device> device,
		void* data, DXGI_FORMAT format, UINT bufferSizeInBytes,
		Microsoft::WRL::ComPtr<ID3D12Resource>& resource, D3D12_INDEX_BUFFER_VIEW& indexView);
	HRESULT CreateHeap(Microsoft::WRL::ComPtr<ID3D12Device> device,
		UINT numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags,
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& resource, UINT& descriptorSize);
	HRESULT CreateConstantBufferView(Microsoft::WRL::ComPtr<ID3D12Device> device,
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap, const UINT heapDescriptorSize, UINT heapOffset,
		UINT numBuffers, UINT bufferSizeInBytes,
		Microsoft::WRL::ComPtr<ID3D12Resource>& resource, CD3DX12_CPU_DESCRIPTOR_HANDLE& handle);
	HRESULT CreateStructuredBufferView(Microsoft::WRL::ComPtr<ID3D12Device> device,
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap, const UINT heapDescriptorSize, UINT heapOffset,
		UINT numAttributes, UINT attributeStride,
		Microsoft::WRL::ComPtr<ID3D12Resource>& resource, CD3DX12_CPU_DESCRIPTOR_HANDLE& handle);
	HRESULT LoadTexture(Microsoft::WRL::ComPtr<ID3D12Device> device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& cmd,
		Microsoft::WRL::ComPtr<ID3D12Resource>& resource, Microsoft::WRL::ComPtr<ID3D12Resource>& upload,
		const std::wstring filepath, bool& IsCubeMap);
	HRESULT LoadDefaultTextures(Microsoft::WRL::ComPtr<ID3D12Device> device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& cmd);
	HRESULT LoadLevelTextures(Microsoft::WRL::ComPtr<ID3D12Device> device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& cmd);

public:
	Renderer(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GDirectX12Surface _d3d);
	VOID Render();
	~Renderer();
	VOID Update(FLOAT deltaTime);

};

LONG_PTR Renderer::gatewareWndProc = 0;

inline LRESULT Renderer::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);

	return CallWindowProcW((WNDPROC)gatewareWndProc, hWnd, msg, wParam, lParam);
}

inline VOID Renderer::DisplayImguiMenu(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmd)
{	
	// Start the Dear ImGui frame
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	GW::SYSTEM::UNIVERSAL_WINDOW_HANDLE uwh;
	win.GetWindowHandle(uwh);


	IM_ASSERT(ImGui::GetCurrentContext() != NULL && "Missing dear imgui context. Refer to examples app!");
	static bool showMainMenu = true;
	ImGui::ShowDemoWindow(&showMainMenu);
	//const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
	//ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 10, main_viewport->WorkPos.y + 10), ImGuiCond_FirstUseEver);
	//ImGui::SetNextWindowSize(ImVec2(50, 50), ImGuiCond_FirstUseEver);

	//ImGuiWindowFlags window_flags = 0;

	//ImGui::Begin("DirectX 12 Project", &showMainMenu, window_flags);
	//{
	//	
	//}
	//ImGui::End();


	// Rendering
	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmd.Get());
	ImGui::EndFrame();
}

inline UINT Renderer::CalculateConstantBufferByteSize(UINT byteSize)
{
	return (byteSize + (D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1)) & ~(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1);
};

inline std::string Renderer::ShaderAsString(const CHAR* shaderFilePath)
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

inline VOID Renderer::UpdateCamera(FLOAT deltaTime)
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
		worldCamera.row4 = { 0.0f, 0.0f, 0.0f, 1.0f };
		matrixProxy.MultiplyMatrixF(worldCamera, y_rotation, worldCamera);
		worldCamera.row4 = position;
	}

	// end of camera update
	matrixProxy.InverseF(worldCamera, viewMatrix);
	matrixProxy.ProjectionDirectXLHF(G2D_DEGREE_TO_RADIAN(65),
		aspect_ratio, 0.1f, 1000.0f,
		projectionMatrix);


	bool textureOn = kbmState[G_KEY_T] || controllerState[G_LEFT_SHOULDER_BTN];
	bool textureOff = kbmState[G_KEY_Y] || controllerState[G_RIGHT_SHOULDER_BTN];
	if (textureOn)
	{
		textureBitMask = 0x00000000u;
	}
	if (textureOff)
	{
		textureBitMask = 0x00000006u;
	}
}

inline VOID Renderer::ReleaseLevelResources()
{
	vertexView = { 0 };
	indexView = { 0 };

	D3D12_COMPTR_SAFE_RELEASE(vertexBuffer);
	D3D12_COMPTR_SAFE_RELEASE(indexBuffer);
	D3D12_COMPTR_SAFE_RELEASE(constantBufferScene);
	D3D12_COMPTR_SAFE_RELEASE(structuredBufferAttributesResource);
	D3D12_COMPTR_SAFE_RELEASE(structuredBufferInstanceResource);
	D3D12_COMPTR_SAFE_RELEASE(structuredBufferLightResource);
	UINT resourceSize = textureResourceDiffuse.size();
	for (UINT i = 0; i < resourceSize; i++)
	{
		D3D12_COMPTR_SAFE_RELEASE(textureResourceDiffuse[i]);
		D3D12_COMPTR_SAFE_RELEASE(textureResourceDiffuseUpload[i]);
	}
	textureResourceDiffuse.clear();
	textureResourceDiffuseUpload.clear();

	resourceSize = textureResourceNormal.size();
	for (UINT i = 0; i < resourceSize; i++)
	{
		D3D12_COMPTR_SAFE_RELEASE(textureResourceNormal[i]);
		D3D12_COMPTR_SAFE_RELEASE(textureResourceNormalUpload[i]);
	}
	textureResourceNormal.clear();
	textureResourceNormalUpload.clear();

	resourceSize = textureResourceSpecular.size();
	for (UINT i = 0; i < resourceSize; i++)
	{
		D3D12_COMPTR_SAFE_RELEASE(textureResourceSpecular[i]);
		D3D12_COMPTR_SAFE_RELEASE(textureResourceSpecularUpload[i]);
	}
	textureResourceSpecular.clear();
	textureResourceSpecularUpload.clear();

	D3D12_COMPTR_SAFE_RELEASE(textureResourceDefault2D);
	D3D12_COMPTR_SAFE_RELEASE(textureResourceDefault2DUpload);
	D3D12_COMPTR_SAFE_RELEASE(textureResourceDefault3D);
	D3D12_COMPTR_SAFE_RELEASE(textureResourceDefault3DUpload);

	constantBufferSceneData = nullptr;
	structuredBufferAttributesData = nullptr;
	structuredBufferInstanceData = nullptr;
	structuredBufferLightData = nullptr;
}

inline BOOL Renderer::LoadLevelDataFromFile(const std::string& filename)
{
	ID3D12Device* device = nullptr;
	ID3D12GraphicsCommandList* cmd = nullptr;
	ID3D12CommandAllocator* allocator = nullptr;
	ID3D12CommandQueue* queue = nullptr;
	d3d.GetDevice((void**)&device);
	d3d.GetCommandList((void**)&cmd);
	d3d.GetCommandAllocator((void**)&allocator);
	d3d.GetCommandQueue((void**)&queue);

	if (!currentLevel.LoadLevel(filename))
	{
		return FALSE;
	}

	worldCamera = currentLevel.camera;
	textureBitMask = 0x00000000u;

	HRESULT hr = E_NOTIMPL;
	{
		////////////////////////////////////////////////////////////////////////////////////////////////////
		// vertex buffer creation
		UINT vertexBufferStride = sizeof(H2B::VERTEX);
		UINT vertexBufferSize = vertexBufferStride * currentLevel.vertex_count;
		void* vertexBufferData = currentLevel.vertices.data();
		hr = LoadVertexData(device,
			vertexBufferData, vertexBufferStride, vertexBufferSize,
			vertexBuffer, vertexView);
		if (FAILED(hr))
		{
			abort();
		}
		////////////////////////////////////////////////////////////////////////////////////////////////////
	}
	{
		////////////////////////////////////////////////////////////////////////////////////////////////////
		// index buffer creation
		DXGI_FORMAT indexBufferFormat = DXGI_FORMAT_R32_UINT;
		UINT indexBufferSize = sizeof(UINT) * currentLevel.index_count;
		void* indexBufferData = currentLevel.indices.data();
		hr = LoadIndexData(device,
			indexBufferData, indexBufferFormat, indexBufferSize,
			indexBuffer, indexView);
		if (FAILED(hr))
		{
			abort();
		}
		////////////////////////////////////////////////////////////////////////////////////////////////////
	}
	// constant buffer (CBV) / structured buffer (SRV) creation
	{
		////////////////////////////////////////////////////////////////////////////////////////////////////
		// constant buffer heap creation
		hr = CreateHeap(device, 5 + MAX_COLOR_TEXTURES + MAX_NORMAL_TEXTURES + MAX_SPECULAR_TEXTURES,
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
			cbvsrvuavHeap, cbvDescriptorSize);
		if (FAILED(hr))
		{
			abort();
		}
		////////////////////////////////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////////////////////////////////////
		hr = CreateConstantBufferView(device, cbvsrvuavHeap, cbvDescriptorSize, 0,
			1, sizeof(SCENE), constantBufferScene, cbvSceneHandle);
		if (FAILED(hr))
		{
			//abort();
		}
		////////////////////////////////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////////////////////////////////////
		// materials
		const UINT numAttributes = currentLevel.materials.GetMaterialCount();
		hr = CreateStructuredBufferView(device, cbvsrvuavHeap, cbvDescriptorSize, 1,
			numAttributes, sizeof(H2B::ATTRIBUTES), structuredBufferAttributesResource, structuredBufferAttributeHandle);
		if (FAILED(hr))
		{
			//abort();
		}
		////////////////////////////////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////////////////////////////////////
		// instance data for matrices
		const UINT numInstances = currentLevel.instanceData.size() + 1;
		hr = CreateStructuredBufferView(device, cbvsrvuavHeap, cbvDescriptorSize, 2,
			numInstances, sizeof(GW::MATH::GMATRIXF), structuredBufferInstanceResource, structuredBufferInstanceHandle);
		if (FAILED(hr))
		{
			//abort();
		}
		////////////////////////////////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////////////////////////////////////
		// lighting information
		const UINT numLights = currentLevel.uniqueLights.size();
		hr = CreateStructuredBufferView(device, cbvsrvuavHeap, cbvDescriptorSize, 3,
			numLights, sizeof(H2B::LIGHT), structuredBufferLightResource, structuredBufferLightHandle);
		if (FAILED(hr))
		{
			//abort();
		}
		////////////////////////////////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////////////////////////////////////
		// Texture Loading 
		// Reset the command allocator and the graphics command list
		// we plan on entering commands to upload information to the gpu
		allocator->Reset();
		cmd->Reset(allocator, nullptr);

		Microsoft::WRL::ComPtr<ID3D12Device> d(device);
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> c(cmd);

		LoadDefaultTextures(d, c);

		LoadLevelTextures(d, c);


		cmd->Close();
		Microsoft::WRL::ComPtr<ID3D12CommandList> lists[] = { cmd };
		queue->ExecuteCommandLists(ARRAYSIZE(lists), lists->GetAddressOf());
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
				memcpy(structuredBufferAttributesData, currentLevel.materials.GetMaterials().data(), sizeof(H2B::ATTRIBUTES) * numAttributes);
				structuredBufferAttributesResource->Unmap(0, nullptr);
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
	cmd->Release();
	allocator->Release();
	queue->Release();

	return TRUE;
}

inline bool Renderer::OpenFileDialogBox(GW::SYSTEM::UNIVERSAL_WINDOW_HANDLE windowHandle, std::string& fileName)
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
		//setup converter
		using convert_type = std::codecvt_utf8<wchar_t>;
		std::wstring_convert<convert_type, wchar_t> converter;
		//use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
		fileName = converter.to_bytes(ws);
		//fileName = std::string(ws.begin(), ws.end());
		result = true;
	}
	return result;
}

// Wait for pending GPU work to complete.
inline VOID Renderer::WaitForGpu()
{
	ID3D12CommandQueue* commandQueue = nullptr;
	ID3D12Fence* fence = nullptr;
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

inline HRESULT Renderer::LoadVertexData(Microsoft::WRL::ComPtr<ID3D12Device> device,
	void* data, UINT strideInBytes, UINT bufferSizeInBytes,
	Microsoft::WRL::ComPtr<ID3D12Resource>& resource, D3D12_VERTEX_BUFFER_VIEW& vertexView)
{
	HRESULT hr = E_NOTIMPL;
	UINT vertexBufferSize = bufferSizeInBytes;
	hr = device->CreateCommittedResource( // using UPLOAD heap for simplicity
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // DEFAULT recommend  
		D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(resource.ReleaseAndGetAddressOf()));
	if (FAILED(hr)) return hr;

	UINT8* transferMemoryLocation = nullptr;
	hr = resource->Map(0, &CD3DX12_RANGE(0, 0), reinterpret_cast<void**>(&transferMemoryLocation));
	if (FAILED(hr)) return hr;
	memcpy(transferMemoryLocation, data, vertexBufferSize);
	resource->Unmap(0, nullptr);
	// Create a vertex View to send to a Draw() call.
	vertexView.BufferLocation = resource->GetGPUVirtualAddress();
	vertexView.StrideInBytes = strideInBytes;
	vertexView.SizeInBytes = vertexBufferSize;
	return hr;
}

inline HRESULT Renderer::LoadIndexData(Microsoft::WRL::ComPtr<ID3D12Device> device,
	void* data, DXGI_FORMAT format, UINT bufferSizeInBytes,
	Microsoft::WRL::ComPtr<ID3D12Resource>& resource, D3D12_INDEX_BUFFER_VIEW& indexView)
{
	HRESULT hr = E_NOTIMPL;
	UINT indexBufferSize = bufferSizeInBytes;
	hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(resource.ReleaseAndGetAddressOf()));
	if (FAILED(hr)) return hr;

	UINT8* transferMemoryLocation = nullptr;
	hr = resource->Map(0, &CD3DX12_RANGE(0, 0), reinterpret_cast<void**>(&transferMemoryLocation));
	if (FAILED(hr)) return hr;
	memcpy(transferMemoryLocation, data, indexBufferSize);
	resource->Unmap(0, nullptr);
	indexView.BufferLocation = resource->GetGPUVirtualAddress();
	indexView.Format = format;
	indexView.SizeInBytes = indexBufferSize;
	return hr;
}

inline HRESULT Renderer::CreateHeap(Microsoft::WRL::ComPtr<ID3D12Device> device,
	UINT numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags,
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& resource, UINT& descriptorSize)
{
	HRESULT hr = E_NOTIMPL;
	////////////////////////////////////////////////////////////////////////////////////////////////////
	// heap creation
	D3D12_DESCRIPTOR_HEAP_DESC cbvsrvuavHeapDesc = {};
	cbvsrvuavHeapDesc.NumDescriptors = numDescriptors;
	cbvsrvuavHeapDesc.Type = type;
	cbvsrvuavHeapDesc.Flags = flags;
	hr = device->CreateDescriptorHeap(&cbvsrvuavHeapDesc, IID_PPV_ARGS(resource.ReleaseAndGetAddressOf()));

	descriptorSize = device->GetDescriptorHandleIncrementSize(type);
	////////////////////////////////////////////////////////////////////////////////////////////////////
	return hr;
}

inline HRESULT Renderer::CreateConstantBufferView(Microsoft::WRL::ComPtr<ID3D12Device> device,
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap, const UINT heapDescriptorSize, UINT heapOffset,
	UINT numBuffers, UINT bufferSizeInBytes,
	Microsoft::WRL::ComPtr<ID3D12Resource>& resource, CD3DX12_CPU_DESCRIPTOR_HANDLE& handle)
{
	HRESULT hr = E_NOTIMPL;
	////////////////////////////////////////////////////////////////////////////////////////////////////
	CD3DX12_HEAP_PROPERTIES prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC resource_desc = CD3DX12_RESOURCE_DESC();
	UINT constantBufferSize = CalculateConstantBufferByteSize(bufferSizeInBytes) * numBuffers;
	resource_desc = CD3DX12_RESOURCE_DESC::Buffer(constantBufferSize);
	// commited resource for the constant buffer
	hr = device->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&resource_desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(resource.ReleaseAndGetAddressOf()));
	if (FAILED(hr)) return hr;

	handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(heap->GetCPUDescriptorHandleForHeapStart(), heapOffset, heapDescriptorSize);

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = resource->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = constantBufferSize;    // CB size is required to be 256-byte aligned.
	// create CBV
	device->CreateConstantBufferView(&cbvDesc, handle);
	////////////////////////////////////////////////////////////////////////////////////////////////////
	return hr;
}

inline HRESULT Renderer::CreateStructuredBufferView(Microsoft::WRL::ComPtr<ID3D12Device> device,
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap, const UINT heapDescriptorSize, UINT heapOffset,
	UINT numAttributes, UINT attributeStride,
	Microsoft::WRL::ComPtr<ID3D12Resource>& resource, CD3DX12_CPU_DESCRIPTOR_HANDLE& handle)
{
	HRESULT hr = E_NOTIMPL;
	//const UINT numAttributes = currentLevel.materials.GetMaterialCount();
	if (numAttributes > 0)
	{
		// commited resource for the structured buffer
		CD3DX12_HEAP_PROPERTIES prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC resource_desc = CD3DX12_RESOURCE_DESC();
		resource_desc = CD3DX12_RESOURCE_DESC::Buffer(attributeStride * numAttributes);
		hr = device->CreateCommittedResource(
			&prop,
			D3D12_HEAP_FLAG_NONE,
			&resource_desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(resource.ReleaseAndGetAddressOf()));
		if (FAILED(hr)) return hr;

		handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(heap->GetCPUDescriptorHandleForHeapStart(), heapOffset, heapDescriptorSize);

		// srv description
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = numAttributes;
		srvDesc.Buffer.StructureByteStride = attributeStride;
		// create SRV
		device->CreateShaderResourceView(resource.Get(), &srvDesc, handle);
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////
	return hr;
}

inline HRESULT Renderer::LoadTexture(Microsoft::WRL::ComPtr<ID3D12Device> device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& cmd,
	Microsoft::WRL::ComPtr<ID3D12Resource>& resource, Microsoft::WRL::ComPtr<ID3D12Resource>& upload,
	const std::wstring filepath, bool& IsCubeMap)
{
	HRESULT hr = E_NOTIMPL;

	std::unique_ptr<uint8_t[]> ddsData;
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	DirectX::DDS_ALPHA_MODE alphaMode = DirectX::DDS_ALPHA_MODE::DDS_ALPHA_MODE_UNKNOWN;
	hr = DirectX::LoadDDSTextureFromFile(device.Get(), filepath.c_str(), resource.ReleaseAndGetAddressOf(),
		ddsData, subresources, 0Ui64, &alphaMode, &IsCubeMap);
	if (FAILED(hr))
	{
		return hr;
	}

	D3D12_RESOURCE_DESC resourceDesc = resource->GetDesc();
	CD3DX12_HEAP_PROPERTIES upload_prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	UINT64 uploadSize = GetRequiredIntermediateSize(resource.Get(), 0, resourceDesc.MipLevels * resourceDesc.DepthOrArraySize);
	CD3DX12_RESOURCE_DESC upload_resource = CD3DX12_RESOURCE_DESC::Buffer(uploadSize);

	// create a heap for uploading
	hr = device->CreateCommittedResource(
		&upload_prop,
		D3D12_HEAP_FLAG_NONE,
		&upload_resource,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(upload.ReleaseAndGetAddressOf()));
	if (FAILED(hr))
	{
		return hr;
	}

	// update the resource using the upload heap
	UINT64 n = UpdateSubresources(cmd.Get(),
		resource.Get(), upload.Get(),
		0, 0, resourceDesc.MipLevels * resourceDesc.DepthOrArraySize,
		subresources.data());

	CD3DX12_RESOURCE_BARRIER resource_barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	cmd->ResourceBarrier(1, &resource_barrier);

	cmd->DiscardResource(upload.Get(), nullptr);

	return S_OK;
}

inline HRESULT Renderer::LoadDefaultTextures(Microsoft::WRL::ComPtr<ID3D12Device> device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& cmd)
{
	constexpr UINT cubemapOffset = 4;
	constexpr UINT colorOffset = 5;
	constexpr UINT normalOffset = colorOffset + MAX_COLOR_TEXTURES;
	HRESULT hr = E_NOTIMPL;
	bool IsCubeMap = false;

	hr = LoadTexture(device, cmd, textureResourceDefault3D, textureResourceDefault3DUpload, L"../textures/uvmapping_3d.dds", IsCubeMap);
	if (SUCCEEDED(hr))
	{
		D3D12_RESOURCE_DESC resourceDesc = textureResourceDefault3D->GetDesc();
		D3D12_SHADER_RESOURCE_VIEW_DESC ddsSrvDesc = D3D12_SHADER_RESOURCE_VIEW_DESC();
		ddsSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		ddsSrvDesc.Format = resourceDesc.Format;
		ddsSrvDesc.ViewDimension = (IsCubeMap) ? D3D12_SRV_DIMENSION_TEXTURECUBE : D3D12_SRV_DIMENSION_TEXTURE2D;
		ddsSrvDesc.TextureCube.MipLevels = resourceDesc.MipLevels;
		CD3DX12_CPU_DESCRIPTOR_HANDLE descHandle(cbvsrvuavHeap->GetCPUDescriptorHandleForHeapStart(), cubemapOffset, cbvDescriptorSize);
		device->CreateShaderResourceView(textureResourceDefault3D.Get(), &ddsSrvDesc, descHandle);
	}
	hr = LoadTexture(device, cmd, textureResourceDefault2D, textureResourceDefault2DUpload, L"../textures/uvmapping_2d.dds", IsCubeMap);
	if (SUCCEEDED(hr))
	{
		D3D12_RESOURCE_DESC resourceDesc = textureResourceDefault2D->GetDesc();
		D3D12_SHADER_RESOURCE_VIEW_DESC ddsSrvDesc = D3D12_SHADER_RESOURCE_VIEW_DESC();
		ddsSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		ddsSrvDesc.Format = resourceDesc.Format;
		ddsSrvDesc.ViewDimension = (IsCubeMap) ? D3D12_SRV_DIMENSION_TEXTURECUBE : D3D12_SRV_DIMENSION_TEXTURE2D;
		ddsSrvDesc.TextureCube.MipLevels = resourceDesc.MipLevels;
		for (size_t i = 0; i < MAX_COLOR_TEXTURES + MAX_NORMAL_TEXTURES + MAX_SPECULAR_TEXTURES; i++)
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE descHandle(cbvsrvuavHeap->GetCPUDescriptorHandleForHeapStart(), colorOffset + i, cbvDescriptorSize);
			device->CreateShaderResourceView(textureResourceDefault2D.Get(), &ddsSrvDesc, descHandle);
		}
	}
	return hr;
}

inline HRESULT Renderer::LoadLevelTextures(Microsoft::WRL::ComPtr<ID3D12Device> device, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& cmd)
{
	HRESULT hr = E_NOTIMPL;
	UINT numTexturesColor = currentLevel.textures.GetTextureColorCount();
	if (numTexturesColor > 0)
	{
		constexpr UINT skyboxOffset = 4;
		constexpr UINT heapOffset = 5;

		textureResourceDiffuse.resize(numTexturesColor);
		textureResourceDiffuseUpload.resize(numTexturesColor);
		bool* IsCubeMap = new bool[numTexturesColor];
		memset(IsCubeMap, 0, sizeof(bool) * numTexturesColor);


		std::vector<std::string> names = currentLevel.textures.GetTexturesColor();
		for (UINT i = 0; i < numTexturesColor; i++)
		{
			std::string current_name = "../textures/" + names[i].substr(0, names[i].size() - 3) + "dds";
			//std::wstring wide_string(current_name.begin(), current_name.end());

			//setup converter
			using convert_type = std::codecvt_utf8<wchar_t>;
			std::wstring_convert<convert_type, wchar_t> converter;
			//use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
			std::wstring wide_string = converter.from_bytes(current_name);
			//fileName = std::string(ws.begin(), ws.end());

			hr = LoadTexture(device, cmd, textureResourceDiffuse[i], textureResourceDiffuseUpload[i], wide_string, IsCubeMap[i]);
			if (SUCCEEDED(hr))
			{
				D3D12_RESOURCE_DESC resourceDesc = textureResourceDiffuse[i]->GetDesc();
				D3D12_SHADER_RESOURCE_VIEW_DESC ddsSrvDesc = D3D12_SHADER_RESOURCE_VIEW_DESC();
				ddsSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				ddsSrvDesc.Format = resourceDesc.Format;
				ddsSrvDesc.ViewDimension = (IsCubeMap[i]) ? D3D12_SRV_DIMENSION_TEXTURECUBE : D3D12_SRV_DIMENSION_TEXTURE2D;

				if (IsCubeMap[i])
				{
					ddsSrvDesc.TextureCube.MipLevels = resourceDesc.MipLevels;
					CD3DX12_CPU_DESCRIPTOR_HANDLE descHandle(cbvsrvuavHeap->GetCPUDescriptorHandleForHeapStart(), skyboxOffset, cbvDescriptorSize);
					device->CreateShaderResourceView(textureResourceDiffuse[i].Get(), &ddsSrvDesc, descHandle);
				}
				else
				{
					ddsSrvDesc.Texture2D.MipLevels = resourceDesc.MipLevels;
					CD3DX12_CPU_DESCRIPTOR_HANDLE descHandle(cbvsrvuavHeap->GetCPUDescriptorHandleForHeapStart(), heapOffset + i, cbvDescriptorSize);
					device->CreateShaderResourceView(textureResourceDiffuse[i].Get(), &ddsSrvDesc, descHandle);
				}
			}
		}

		if (IsCubeMap)
		{
			delete[] IsCubeMap;
			IsCubeMap = nullptr;
		}
	}

	UINT numTexturesNormal = currentLevel.textures.GetTextureNormalCount();
	if (numTexturesNormal > 0)
	{
		UINT heapOffset = 5 + MAX_COLOR_TEXTURES;

		textureResourceNormal.resize(numTexturesNormal);
		textureResourceNormalUpload.resize(numTexturesNormal);
		bool* IsCubeMap = new bool[numTexturesNormal];
		memset(IsCubeMap, 1, sizeof(bool) * numTexturesNormal);

		std::vector<std::string> names = currentLevel.textures.GetTexturesNormal();
		for (UINT i = 0; i < numTexturesNormal; i++)
		{
			std::string current_name = "../textures/" + names[i].substr(0, names[i].size() - 3) + "dds";
			std::wstring wide_string(current_name.begin(), current_name.end());

			hr = LoadTexture(device, cmd, textureResourceNormal[i], textureResourceNormalUpload[i], wide_string, IsCubeMap[i]);
			if (SUCCEEDED(hr))
			{
				D3D12_RESOURCE_DESC resourceDesc = textureResourceNormal[i]->GetDesc();
				D3D12_SHADER_RESOURCE_VIEW_DESC ddsSrvDesc = D3D12_SHADER_RESOURCE_VIEW_DESC();
				ddsSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				ddsSrvDesc.Format = resourceDesc.Format;
				ddsSrvDesc.ViewDimension = (IsCubeMap[i]) ? D3D12_SRV_DIMENSION_TEXTURECUBE : D3D12_SRV_DIMENSION_TEXTURE2D;
				ddsSrvDesc.Texture2D.MipLevels = resourceDesc.MipLevels;
				CD3DX12_CPU_DESCRIPTOR_HANDLE descHandle(cbvsrvuavHeap->GetCPUDescriptorHandleForHeapStart(), heapOffset + i, cbvDescriptorSize);
				device->CreateShaderResourceView(textureResourceNormal[i].Get(), &ddsSrvDesc, descHandle);
			}
		}
		if (IsCubeMap)
		{
			delete[] IsCubeMap;
			IsCubeMap = nullptr;
		}
	}

	UINT numTexturesSpecular = currentLevel.textures.GetTextureSpecularCount();
	if (numTexturesSpecular > 0)
	{
		UINT heapOffset = 5 + MAX_COLOR_TEXTURES + MAX_NORMAL_TEXTURES;

		textureResourceSpecular.resize(numTexturesSpecular);
		textureResourceSpecularUpload.resize(numTexturesSpecular);
		bool* IsCubeMap = new bool[numTexturesSpecular];
		memset(IsCubeMap, 1, sizeof(bool) * numTexturesSpecular);

		std::vector<std::string> names = currentLevel.textures.GetTexturesSpecular();
		for (UINT i = 0; i < numTexturesSpecular; i++)
		{
			std::string current_name = "../textures/" + names[i].substr(0, names[i].size() - 3) + "dds";
			std::wstring wide_string(current_name.begin(), current_name.end());

			hr = LoadTexture(device, cmd, textureResourceSpecular[i], textureResourceSpecularUpload[i], wide_string, IsCubeMap[i]);
			if (SUCCEEDED(hr))
			{
				D3D12_RESOURCE_DESC resourceDesc = textureResourceSpecular[i]->GetDesc();
				D3D12_SHADER_RESOURCE_VIEW_DESC ddsSrvDesc = D3D12_SHADER_RESOURCE_VIEW_DESC();
				ddsSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				ddsSrvDesc.Format = resourceDesc.Format;
				ddsSrvDesc.ViewDimension = (IsCubeMap[i]) ? D3D12_SRV_DIMENSION_TEXTURECUBE : D3D12_SRV_DIMENSION_TEXTURE2D;
				ddsSrvDesc.Texture2D.MipLevels = resourceDesc.MipLevels;
				CD3DX12_CPU_DESCRIPTOR_HANDLE descHandle(cbvsrvuavHeap->GetCPUDescriptorHandleForHeapStart(), heapOffset + i, cbvDescriptorSize);
				device->CreateShaderResourceView(textureResourceSpecular[i].Get(), &ddsSrvDesc, descHandle);
			}
		}

		if (IsCubeMap)
		{
			delete[] IsCubeMap;
			IsCubeMap = nullptr;
		}
	}

	return hr;
}

Renderer::Renderer(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GDirectX12Surface _d3d)
{
	dialogBoxOpen = false;
	fenceValues = 0;

	win = _win;
	d3d = _d3d;
	ID3D12Device* device;
	d3d.GetDevice((void**)&device);

	matrixProxy.Create();
	kbmProxy.Create(win);
	controllerProxy.Create();
	bufferedInput.Create(win);

	GW::SYSTEM::UNIVERSAL_WINDOW_HANDLE uwh;
	+win.GetWindowHandle(uwh);

	//internal_gw::GInputGlobal()._userWinProc = SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)GWinProc);
	gatewareWndProc = SetWindowLongPtr((HWND)uwh.window, GWLP_WNDPROC, (LONG_PTR)WndProc);

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


	std::string vertexShaders[] =
	{
		"../shaders/vertexshader.hlsl",
		"../shaders/vertexshaderskybox.hlsl"
	};
	std::string pixelShaders[] =
	{
		"../shaders/pixelshader.hlsl",
		"../shaders/pixelshaderskybox.hlsl"
	};

	// Create Vertex Shader
	std::vector<Microsoft::WRL::ComPtr<ID3DBlob>> vsBlob;
	std::vector<Microsoft::WRL::ComPtr<ID3DBlob>> vsErrors;
	std::vector<Microsoft::WRL::ComPtr<ID3DBlob>> psBlob;
	std::vector<Microsoft::WRL::ComPtr<ID3DBlob>> psErrors;

	vsBlob.resize(ARRAYSIZE(vertexShaders));
	vsErrors.resize(ARRAYSIZE(vertexShaders));
	psBlob.resize(ARRAYSIZE(pixelShaders));
	psErrors.resize(ARRAYSIZE(pixelShaders));

	for (UINT i = 0; i < ARRAYSIZE(vertexShaders); i++)
	{
		std::string vertexShaderString = ShaderAsString(vertexShaders[i].c_str());
		if (FAILED(D3DCompile(vertexShaderString.c_str(), vertexShaderString.length(),
			nullptr, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_1", compilerFlags, 0,
			vsBlob[i].ReleaseAndGetAddressOf(), vsErrors[i].ReleaseAndGetAddressOf())))
		{
			std::cout << (char*)vsErrors[i]->GetBufferPointer() << std::endl;
			abort();
		}
	}
	// Create Pixel Shader
	for (UINT i = 0; i < ARRAYSIZE(pixelShaders); i++)
	{
		std::string pixelShaderString = ShaderAsString(pixelShaders[i].c_str());
		if (FAILED(D3DCompile(pixelShaderString.c_str(), pixelShaderString.length(),
			nullptr, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_1", compilerFlags, 0,
			psBlob[i].ReleaseAndGetAddressOf(), psErrors[i].ReleaseAndGetAddressOf())))
		{
			std::cout << (char*)psErrors[i]->GetBufferPointer() << std::endl;
			abort();
		}
	}
	// Create Input Layout
	D3D12_INPUT_ELEMENT_DESC format[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	CD3DX12_DESCRIPTOR_RANGE1 ranges[4] = {};
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
	ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, -1, 0, 1, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
	ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, -1, 0, 2, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
	ranges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, -1, 0, 3, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);

	CD3DX12_ROOT_PARAMETER1 rootParameters[9] = {};
	rootParameters[0].InitAsConstants(6, 0, 0);					// num32bitConstants, register, space  (mesh_id)	b0, spaec0
	rootParameters[1].InitAsConstantBufferView(1, 0);			// register, space	(view,projection)				b1, spaec0
	rootParameters[2].InitAsShaderResourceView(0, 0);			// register, space	(OBJ_ATTRIBUTES)				t0, spaec0
	rootParameters[3].InitAsShaderResourceView(1, 0);			// register, space	(instance matrix data)			t1, spaec0
	rootParameters[4].InitAsShaderResourceView(2, 0);			// register, space	(light data)					t2, spaec0
	rootParameters[5].InitAsDescriptorTable(1, &ranges[0]);		// count, table(s) (skybox texture)					t3, space0
	rootParameters[6].InitAsDescriptorTable(1, &ranges[1]);		// count, table(s) (color textures)					t0, space1
	rootParameters[7].InitAsDescriptorTable(1, &ranges[2]);		// count, table(s) (normal textures)				t0, space2
	rootParameters[8].InitAsDescriptorTable(1, &ranges[3]);		// count, table(s) (specular textures)				t0, space3

	// static samplers
	CD3DX12_STATIC_SAMPLER_DESC sampler = CD3DX12_STATIC_SAMPLER_DESC(
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
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc(
		ARRAYSIZE(rootParameters), rootParameters,
		1, &sampler,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	Microsoft::WRL::ComPtr<ID3DBlob> signature, errors;
	hr = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1_1, &signature, &errors);
	if (FAILED(hr))
	{
		std::cout << (char*)errors->GetBufferPointer() << std::endl;
		abort();
	}

	device->CreateRootSignature(0, signature->GetBufferPointer(),
		signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature));

	// create pipeline state
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psDesc = {};
	psDesc.InputLayout = { format, ARRAYSIZE(format) };
	psDesc.pRootSignature = rootSignature.Get();
	psDesc.VS = CD3DX12_SHADER_BYTECODE(vsBlob[0].Get());
	psDesc.PS = CD3DX12_SHADER_BYTECODE(psBlob[0].Get());
	psDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psDesc.SampleMask = UINT_MAX;
	psDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psDesc.NumRenderTargets = 1;
	psDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psDesc.SampleDesc.Count = 1;
	hr = device->CreateGraphicsPipelineState(&psDesc, IID_PPV_ARGS(&pipeline));
	if (FAILED(hr))
	{
		abort();
	}

	// create pipeline state
	psDesc.VS = CD3DX12_SHADER_BYTECODE(vsBlob[1].Get());
	psDesc.PS = CD3DX12_SHADER_BYTECODE(psBlob[1].Get());
	psDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	hr = device->CreateGraphicsPipelineState(&psDesc, IID_PPV_ARGS(&pipelineSkybox));
	if (FAILED(hr))
	{
		abort();
	}


	UINT descSize = 0;
	hr = CreateHeap(device, 1,
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
		imguiSrvDescHeap, descSize);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	// Setup Platform/Renderer backends
	bool success = false;
	success = ImGui_ImplWin32_Init(uwh.window);
	success = ImGui_ImplDX12_Init(device, 2,
		DXGI_FORMAT_R8G8B8A8_UNORM, imguiSrvDescHeap.Get(),
		imguiSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
		imguiSrvDescHeap->GetGPUDescriptorHandleForHeapStart());

	// free temporary handle
	device->Release();
}

Renderer::~Renderer()
{
	// Cleanup
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	// ComPtr will auto release so nothing to do here 
	if (constantBufferScene)
	{
		constantBufferScene->Unmap(0, nullptr);
	}

	if (structuredBufferInstanceResource)
	{
		structuredBufferInstanceResource->Unmap(0, nullptr);
	}

	ReleaseLevelResources();

	//ID3D12Device* device = nullptr;
	//d3d.GetDevice((void**)&device);
	//Microsoft::WRL::ComPtr<ID3D12DebugDevice> debug = nullptr;
	//if (device)
	//{
	//	if (SUCCEEDED(device->QueryInterface(debug.ReleaseAndGetAddressOf())))
	//	{
	//		debug->ReportLiveDeviceObjects(D3D12_RLDO_FLAGS::D3D12_RLDO_IGNORE_INTERNAL);
	//	}
	//	device->Release();
	//}
}

VOID Renderer::Render()
{
	// grab the context & render target
	ID3D12GraphicsCommandList1* cmd;
	D3D12_CPU_DESCRIPTOR_HANDLE rtv;
	D3D12_CPU_DESCRIPTOR_HANDLE dsv;
	d3d.GetCommandList((void**)&cmd);
	d3d.GetCurrentRenderTargetView((void**)&rtv);
	d3d.GetDepthStencilView((void**)&dsv);
	// setup the pipeline
	cmd->OMSetRenderTargets(1, &rtv, FALSE, &dsv);
	cmd->SetGraphicsRootSignature(rootSignature.Get());
	cmd->SetPipelineState(pipeline.Get());

	// if the currentlevel is loaded then process the level information
	if (vertexView.BufferLocation && indexView.BufferLocation)
	{
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> ppHeaps[] = { cbvsrvuavHeap.Get() };
		cmd->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps->GetAddressOf());

		// now we can draw
		cmd->IASetVertexBuffers(0, 1, &vertexView);
		cmd->IASetIndexBuffer(&indexView);
		cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		if (constantBufferScene)
		{
			D3D12_GPU_VIRTUAL_ADDRESS cbvHandle = constantBufferScene->GetGPUVirtualAddress();// +0U * (unsigned long long)CalculateConstantBufferByteSize(sizeof(SCENE));
			cmd->SetGraphicsRootConstantBufferView(1, cbvHandle);
		}
		if (structuredBufferAttributesResource)
		{
			D3D12_GPU_VIRTUAL_ADDRESS srvAttributesHandle = structuredBufferAttributesResource->GetGPUVirtualAddress();// +0U * (sizeof(H2B::ATTRIBUTES));
			cmd->SetGraphicsRootShaderResourceView(2, srvAttributesHandle);
		}
		if (structuredBufferInstanceResource)
		{
			D3D12_GPU_VIRTUAL_ADDRESS srvInstanceHandle = structuredBufferInstanceResource->GetGPUVirtualAddress();// +0U * sizeof(GW::MATH::GMATRIXF);
			cmd->SetGraphicsRootShaderResourceView(3, srvInstanceHandle);
		}
		if (structuredBufferLightResource)
		{
			D3D12_GPU_VIRTUAL_ADDRESS srvLightHandle = structuredBufferLightResource->GetGPUVirtualAddress();// +0U * sizeof(H2B::LIGHT);
			cmd->SetGraphicsRootShaderResourceView(4, srvLightHandle);
		}
		const UINT cubeMapOffset = 4;
		const UINT colorTexturesOffset = 5;
		const UINT normalTexturesOffset = colorTexturesOffset + MAX_COLOR_TEXTURES;
		const UINT specularTexturesOffset = normalTexturesOffset + MAX_NORMAL_TEXTURES;

		if (currentLevel.textures.GetTextureColorCount() > 0)
		{
			CD3DX12_GPU_DESCRIPTOR_HANDLE cubemapSrvHandle(cbvsrvuavHeap->GetGPUDescriptorHandleForHeapStart(), cubeMapOffset, cbvDescriptorSize);
			cmd->SetGraphicsRootDescriptorTable(5, cubemapSrvHandle);
			CD3DX12_GPU_DESCRIPTOR_HANDLE colorTextureSrvHandle(cbvsrvuavHeap->GetGPUDescriptorHandleForHeapStart(), colorTexturesOffset, cbvDescriptorSize);
			cmd->SetGraphicsRootDescriptorTable(6, colorTextureSrvHandle);
		}

		if (currentLevel.textures.GetTextureNormalCount() > 0)
		{
			CD3DX12_GPU_DESCRIPTOR_HANDLE normalTextureSrvHandle(cbvsrvuavHeap->GetGPUDescriptorHandleForHeapStart(), normalTexturesOffset, cbvDescriptorSize);
			cmd->SetGraphicsRootDescriptorTable(7, normalTextureSrvHandle);
		}

		if (currentLevel.textures.GetTextureSpecularCount() > 0)
		{
			CD3DX12_GPU_DESCRIPTOR_HANDLE specularTextureSrvHandle(cbvsrvuavHeap->GetGPUDescriptorHandleForHeapStart(), specularTexturesOffset, cbvDescriptorSize);
			cmd->SetGraphicsRootDescriptorTable(8, specularTextureSrvHandle);
		}

		for (const auto& mesh : currentLevel.uniqueMeshes)	// mesh count
		{
			for (const auto& submesh : mesh.second.subMeshes)	// submesh count
			{
				UINT root32BitConstants[] =
				{
					mesh.second.meshIndex, submesh.materialIndex,
					submesh.hasTexture & ~textureBitMask,
					submesh.colorTextureIndex, submesh.normalTextureIndex, submesh.specularTextureIndex
				};
				cmd->SetGraphicsRoot32BitConstants(0, ARRAYSIZE(root32BitConstants), root32BitConstants, 0);
				cmd->DrawIndexedInstanced(submesh.drawInfo.indexCount, mesh.second.numInstances, submesh.drawInfo.indexOffset, mesh.second.vertexOffset, 0);
			}
		}

		cmd->SetPipelineState(pipelineSkybox.Get());
		for (const auto& mesh : currentLevel.uniqueSkyboxes)	// skybox count
		{
			for (const auto& submesh : mesh.second.subMeshes)	// submesh count
			{
				UINT root32BitConstants[] =
				{
					mesh.second.meshIndex, submesh.materialIndex,
					submesh.hasTexture,
					submesh.colorTextureIndex, submesh.normalTextureIndex, submesh.specularTextureIndex
				};
				cmd->SetGraphicsRoot32BitConstants(0, ARRAYSIZE(root32BitConstants), root32BitConstants, 0);
				cmd->DrawIndexedInstanced(submesh.drawInfo.indexCount, mesh.second.numInstances, submesh.drawInfo.indexOffset, mesh.second.vertexOffset, 0);
			}
		}
	}

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> imguiHeaps[] = { imguiSrvDescHeap.Get() };
	cmd->SetDescriptorHeaps(_countof(imguiHeaps), imguiHeaps->GetAddressOf());

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> c(cmd);
	DisplayImguiMenu(c);

	// release temp handles
	cmd->Release();
}

VOID Renderer::Update(FLOAT deltaTime)
{
	HRESULT hr = E_NOTIMPL;
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

	if (structuredBufferInstanceResource)
	{
		UINT numInstances = currentLevel.instanceData.size() + 1;
		hr = structuredBufferInstanceResource->Map(0, &CD3DX12_RANGE(0, 0), reinterpret_cast<void**>(&structuredBufferInstanceData));
		if (SUCCEEDED(hr))
		{
			if (numInstances - 1 > 0)
			{
				memcpy(structuredBufferInstanceData, currentLevel.instanceData.data(), sizeof(GW::MATH::GMATRIXF) * numInstances);
			}
			GW::MATH::GMATRIXF skyboxWorldMatrix = GW::MATH::GIdentityMatrixF;
			skyboxWorldMatrix.row4 = worldCamera.row4;
			memcpy(&structuredBufferInstanceData[numInstances - 1], &skyboxWorldMatrix, sizeof(GW::MATH::GMATRIXF));
		}
	}
}