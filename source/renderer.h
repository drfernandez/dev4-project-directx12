// minimalistic code to draw a single triangle, this is not part of the API.
// required for compiling shaders on the fly, consider pre-compiling instead
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")
#include "d3dx12.h" // official helper file provided by microsoft
#include "FSLogo.h"


struct VP
{
	GW::MATH::GMATRIXF view;
	GW::MATH::GMATRIXF projection;
};


// Creation, Rendering & Cleanup
class Renderer
{
private:
	// proxy handles
	GW::SYSTEM::GWindow								win;
	GW::GRAPHICS::GDirectX12Surface					d3d;

	D3D12_VERTEX_BUFFER_VIEW						vertexView;
	D3D12_INDEX_BUFFER_VIEW							indexView;
	Microsoft::WRL::ComPtr<ID3D12Resource>			vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource>			indexBuffer;

	Microsoft::WRL::ComPtr<ID3D12RootSignature>		rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState>		pipeline;

	GW::MATH::GMATRIXF*								constantBufferData;
	UINT											cbvDescriptorSize;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	cbvHeap;
	Microsoft::WRL::ComPtr <ID3D12Resource>			constantBuffer;


	inline UINT CalculateConstantBufferByteSize(UINT byteSize)
	{
		// Constant buffer size is required to be aligned.
		return (byteSize + (D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1)) & ~(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1);
	}


public:
	Renderer(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GDirectX12Surface _d3d)
	{
		win = _win;
		d3d = _d3d;
		ID3D12Device* creator;
		d3d.GetDevice((void**)&creator);

		// vertex buffer creation
		{
			creator->CreateCommittedResource( // using UPLOAD heap for simplicity
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // DEFAULT recommend  
				D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(sizeof(FSLogo_vertices)),
				D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexBuffer));

			UINT8* transferMemoryLocation = nullptr;
			vertexBuffer->Map(0, &CD3DX12_RANGE(0, 0),
				reinterpret_cast<void**>(&transferMemoryLocation));
			memcpy(transferMemoryLocation, FSLogo_vertices, sizeof(FSLogo_vertices));
			vertexBuffer->Unmap(0, nullptr);
			// Create a vertex View to send to a Draw() call.
			vertexView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
			vertexView.StrideInBytes = sizeof(OBJ_VERT);
			vertexView.SizeInBytes = sizeof(FSLogo_vertices);
		}
		// index buffer creation
		{
			creator->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(sizeof(FSLogo_indices)),
				D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&indexBuffer));

			UINT8* transferMemoryLocation = nullptr;
			indexBuffer->Map(0, &CD3DX12_RANGE(0, 0),
				reinterpret_cast<void**>(&transferMemoryLocation));
			memcpy(transferMemoryLocation, FSLogo_indices, sizeof(FSLogo_indices));
			indexBuffer->Unmap(0, nullptr);
			indexView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
			indexView.SizeInBytes = sizeof(FSLogo_indices);
			indexView.Format = DXGI_FORMAT_R32_UINT;
		}
		// constant buffer creation
		{
			// constant buffer heap creation
			D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
			cbvHeapDesc.NumDescriptors = 10;		// X number of constant buffer heaps (256 bytes)
			cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			creator->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(cbvHeap.ReleaseAndGetAddressOf()));

			cbvDescriptorSize = creator->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

			HRESULT hr = E_NOTIMPL;
			CD3DX12_HEAP_PROPERTIES prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
			CD3DX12_RESOURCE_DESC resource_desc = CD3DX12_RESOURCE_DESC::Buffer(CalculateConstantBufferByteSize(sizeof(GW::MATH::GMATRIXF) * 12));
			hr = creator->CreateCommittedResource(
				&prop,
				D3D12_HEAP_FLAG_NONE,
				&resource_desc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&constantBuffer));

			CD3DX12_CPU_DESCRIPTOR_HANDLE cbvHandle(cbvHeap->GetCPUDescriptorHandleForHeapStart(), 0, cbvDescriptorSize);
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
			cbvDesc.BufferLocation = constantBuffer->GetGPUVirtualAddress();
			cbvDesc.SizeInBytes = CalculateConstantBufferByteSize(sizeof(GW::MATH::GMATRIXF) * 12);    // CB size is required to be 256-byte aligned.

			creator->CreateConstantBufferView(&cbvDesc, cbvHandle);

			CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
			constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&constantBufferData));
		}

		UINT compilerFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if _DEBUG
		compilerFlags |= D3DCOMPILE_DEBUG;
#endif
		std::string vertexShaderString = ShaderAsString("../shaders/vertexshader.hlsl");
		std::string pixelShaderString = ShaderAsString("../shaders/pixelshader.hlsl");

		// Create Vertex Shader
		Microsoft::WRL::ComPtr<ID3DBlob> vsBlob, errors;
		if (FAILED(D3DCompile(vertexShaderString.c_str(), vertexShaderString.length(),
			nullptr, nullptr, nullptr, "main", "vs_5_0", compilerFlags, 0,
			vsBlob.GetAddressOf(), errors.GetAddressOf())))
		{
			std::cout << (char*)errors->GetBufferPointer() << std::endl;
			abort();
		}
		// Create Pixel Shader
		Microsoft::WRL::ComPtr<ID3DBlob> psBlob; errors.Reset();
		if (FAILED(D3DCompile(pixelShaderString.c_str(), pixelShaderString.length(),
			nullptr, nullptr, nullptr, "main", "ps_5_0", compilerFlags, 0,
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
		

		CD3DX12_ROOT_PARAMETER rootParameters[1] = {};
		rootParameters[0].InitAsConstantBufferView(0, 0); // register, space


		// create root signature
		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
		rootSignatureDesc.Init(
			1, rootParameters, // number of params, root parameters
			0, nullptr, // number of samplers, static samplers
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		Microsoft::WRL::ComPtr<ID3DBlob> signature;
		D3D12SerializeRootSignature(&rootSignatureDesc,
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
	void Render()
	{
		// grab the context & render target
		ID3D12GraphicsCommandList* cmd;
		D3D12_CPU_DESCRIPTOR_HANDLE rtv;
		D3D12_CPU_DESCRIPTOR_HANDLE dsv;
		d3d.GetCommandList((void**)&cmd);
		d3d.GetCurrentRenderTargetView((void**)&rtv);
		d3d.GetDepthStencilView((void**)&dsv);
		// setup the pipeline
		cmd->SetGraphicsRootSignature(rootSignature.Get());
		cmd->OMSetRenderTargets(1, &rtv, FALSE, &dsv);
		cmd->SetPipelineState(pipeline.Get());
		// now we can draw
		D3D12_GPU_VIRTUAL_ADDRESS cbvHandle = constantBuffer->GetGPUVirtualAddress() + 1 * CalculateConstantBufferByteSize(sizeof(GW::MATH::GMATRIXF));
		cmd->IASetVertexBuffers(0, 1, &vertexView);
		cmd->IASetIndexBuffer(&indexView);
		cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		cmd->SetGraphicsRootConstantBufferView(0, cbvHandle);
		cmd->DrawIndexedInstanced(FSLogo_indexcount, 1, 0, 0, 0);
		// release temp handles
		cmd->Release();
	}

	~Renderer()
	{
		// ComPtr will auto release so nothing to do here 

		constantBuffer->Unmap(0, nullptr);
	}

	void Update(float delta)
	{

	}

	std::string Renderer::ShaderAsString(const char* shaderFilePath)
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
};
