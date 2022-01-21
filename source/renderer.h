#pragma once
// Simple basecode showing how to create a window and attatch a d3d12surface
#define GATEWARE_ENABLE_CORE // All libraries need this
#define GATEWARE_ENABLE_SYSTEM // Graphics libs require system level libraries
#define GATEWARE_ENABLE_GRAPHICS // Enables all Graphics Libraries
#define GATEWARE_ENABLE_MATH // Enables all Math Libraries
#define GATEWARE_ENABLE_INPUT
// Ignore some GRAPHICS libraries we aren't going to use
#define GATEWARE_DISABLE_GDIRECTX11SURFACE // we have another template for this
#define GATEWARE_DISABLE_GOPENGLSURFACE // we have another template for this
#define GATEWARE_DISABLE_GVULKANSURFACE // we have another template for this
#define GATEWARE_DISABLE_GRASTERSURFACE // we have another template for this
// With what we want & what we don't defined we can include the API
#include "Gateware.h"

// minimalistic code to draw a single triangle, this is not part of the API.
// required for compiling shaders on the fly, consider pre-compiling instead
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")
#include "d3dx12.h" // official helper file provided by microsoft
//#include "FSLogo.h"
#include "level.h"

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

public:
	Renderer(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GDirectX12Surface _d3d);
	VOID Render();
	~Renderer();
	VOID Update(FLOAT deltaTime);

};