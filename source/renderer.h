// minimalistic code to draw a single triangle, this is not part of the API.
// required for compiling shaders on the fly, consider pre-compiling instead
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")
#include "d3dx12.h" // official helper file provided by microsoft
#include "FSLogo.h"

struct MODEL_DATA
{
	GW::MATH::GMATRIXF      world;
	OBJ_ATTRIBUTES			attribs;
};


// Creation, Rendering & Cleanup
class Renderer
{
private:
	// proxy handles
	GW::SYSTEM::GWindow								win;
	GW::GRAPHICS::GDirectX12Surface					d3d;

	GW::MATH::GMatrix								matrixProxy;
	GW::MATH::GMATRIXF								worldMatrix[2];
	GW::MATH::GMATRIXF								viewMatrix;
	GW::MATH::GMATRIXF								projectionMatrix;

	D3D12_VERTEX_BUFFER_VIEW						vertexView;
	D3D12_INDEX_BUFFER_VIEW							indexView;
	Microsoft::WRL::ComPtr<ID3D12Resource>			vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource>			indexBuffer;

	Microsoft::WRL::ComPtr<ID3D12RootSignature>		rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState>		pipeline;

	UINT											cbvDescriptorSize;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	cbvHeap;
	GW::MATH::GMATRIXF*								constantBufferSceneData;
	Microsoft::WRL::ComPtr<ID3D12Resource>			constantBufferScene;
	CD3DX12_CPU_DESCRIPTOR_HANDLE					cbvSceneHandle;

	MODEL_DATA*										structuredBufferModelData;
	Microsoft::WRL::ComPtr<ID3D12Resource>			structuredBufferModel;
	CD3DX12_CPU_DESCRIPTOR_HANDLE					structuredBufferHandle;

	inline UINT CalculateConstantBufferByteSize(UINT byteSize)
	{
		return (byteSize + (D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1)) & ~(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1);
	};

	inline std::string Renderer::ShaderAsString(const char* shaderFilePath)
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

public:
	Renderer(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GDirectX12Surface _d3d)
	{
		win = _win;
		d3d = _d3d;
		ID3D12Device* creator;
		d3d.GetDevice((void**)&creator);

		matrixProxy.Create();

		for (UINT i = 0; i < ARRAYSIZE(worldMatrix); i++)
		{
			worldMatrix[i] = GW::MATH::GIdentityMatrixF;
		}

		// view and projection creation
		{
			GW::MATH::GVECTORF eye = { 0.75f, 0.25f, -1.5f, 0.0f };
			GW::MATH::GVECTORF at = { 0.15f, 0.75f, 0.0f, 0.0f };
			GW::MATH::GVECTORF up = { 0.0f, 1.0f, 0.0f, 0.0f };
			matrixProxy.LookAtLHF(eye, at, up, viewMatrix);

			float fov = G_DEGREE_TO_RADIAN(65);
			float zn = 0.1f;
			float zf = 100.0f;
			float aspect = 0.0f;
			d3d.GetAspectRatio(aspect);
			matrixProxy.ProjectionDirectXLHF(fov, aspect, zn, zf, projectionMatrix);
		}

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
		// constant buffer (CBV) / structured buffer (SRV) creation
		{
			// constant buffer heap creation
			D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
			cbvHeapDesc.NumDescriptors = 2;		// X number of descriptors to create from cbv_srv_uav heap
			cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			creator->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(cbvHeap.ReleaseAndGetAddressOf()));

			cbvDescriptorSize = creator->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

			HRESULT hr = E_NOTIMPL;
			CD3DX12_HEAP_PROPERTIES prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
			CD3DX12_RESOURCE_DESC resource_desc = CD3DX12_RESOURCE_DESC::Buffer(CalculateConstantBufferByteSize(sizeof(GW::MATH::GMATRIXF) * 2));
			// commited resource for the constant buffer
			hr = creator->CreateCommittedResource(
				&prop,
				D3D12_HEAP_FLAG_NONE,
				&resource_desc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&constantBufferScene));

			cbvSceneHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(cbvHeap->GetCPUDescriptorHandleForHeapStart(), 0, cbvDescriptorSize);

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
			cbvDesc.BufferLocation = constantBufferScene->GetGPUVirtualAddress();
			cbvDesc.SizeInBytes = CalculateConstantBufferByteSize(sizeof(GW::MATH::GMATRIXF) * 2);    // CB size is required to be 256-byte aligned.
			// create CBV
			creator->CreateConstantBufferView(&cbvDesc, cbvSceneHandle);

			// commited resource for the structured buffer
			resource_desc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(MODEL_DATA) * FSLogo_meshcount);
			hr = creator->CreateCommittedResource(
				&prop,
				D3D12_HEAP_FLAG_NONE,
				&resource_desc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&structuredBufferModel));

			// srv description
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.NumElements = FSLogo_meshcount;
			srvDesc.Buffer.StructureByteStride = sizeof(MODEL_DATA);			

			structuredBufferHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(cbvHeap->GetCPUDescriptorHandleForHeapStart(), 1, cbvDescriptorSize);
			// create SRV
			creator->CreateShaderResourceView(structuredBufferModel.Get(), &srvDesc, structuredBufferHandle);

			CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
			hr = constantBufferScene->Map(0, &readRange, reinterpret_cast<void**>(&constantBufferSceneData));
			hr = structuredBufferModel->Map(0, &readRange, reinterpret_cast<void**>(&structuredBufferModelData));
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
			nullptr, nullptr, nullptr, "main", "vs_5_1", compilerFlags, 0,
			vsBlob.GetAddressOf(), errors.GetAddressOf())))
		{
			std::cout << (char*)errors->GetBufferPointer() << std::endl;
			abort();
		}
		// Create Pixel Shader
		Microsoft::WRL::ComPtr<ID3DBlob> psBlob; errors.Reset();
		if (FAILED(D3DCompile(pixelShaderString.c_str(), pixelShaderString.length(),
			nullptr, nullptr, nullptr, "main", "ps_5_1", compilerFlags, 0,
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

		CD3DX12_ROOT_PARAMETER rootParameters[3] = {};
		rootParameters[0].InitAsConstantBufferView(0, 0);	// register, space	(view,projection)
		rootParameters[1].InitAsShaderResourceView(0, 0);	// register, space	(MODEL_DATA)
		rootParameters[2].InitAsConstants(1, 1, 0);			// num32bitConstants, register, space  (mesh_id)
		//rootParameters[3].InitAsDescriptorTable(ARRAYSIZE(ranges), ranges);

		// create root signature
		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
		rootSignatureDesc.Init(
			ARRAYSIZE(rootParameters), rootParameters, // number of params, root parameters
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
		D3D12_GPU_VIRTUAL_ADDRESS cbvHandle = constantBufferScene->GetGPUVirtualAddress() + 0U * (unsigned long long)CalculateConstantBufferByteSize(sizeof(GW::MATH::GMATRIXF) * 2);
		D3D12_GPU_VIRTUAL_ADDRESS srvHandle = structuredBufferModel->GetGPUVirtualAddress() + 0U * (sizeof(MODEL_DATA) * 2);
		cmd->IASetVertexBuffers(0, 1, &vertexView);
		cmd->IASetIndexBuffer(&indexView);
		cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		cmd->SetGraphicsRootConstantBufferView(0, cbvHandle);
		cmd->SetGraphicsRootShaderResourceView(1, srvHandle);

		for (UINT i = 0; i < FSLogo_meshcount; i++)
		{
			cmd->SetGraphicsRoot32BitConstants(2, 1, &i, 0);
			cmd->DrawIndexedInstanced(FSLogo_meshes[i].indexCount, 1, FSLogo_meshes[i].indexOffset, 0, 0);
		}
		// release temp handles
		cmd->Release();
	}

	~Renderer()
	{
		// ComPtr will auto release so nothing to do here 
		constantBufferScene->Unmap(0, nullptr);
		structuredBufferModel->Unmap(0, nullptr);
	}

	void Update(float deltaTime)
	{
		memcpy(&constantBufferSceneData[0], &viewMatrix, sizeof(GW::MATH::GMATRIXF));
		memcpy(&constantBufferSceneData[1], &projectionMatrix, sizeof(GW::MATH::GMATRIXF));

		matrixProxy.RotateYGlobalF(worldMatrix[1], G_DEGREE_TO_RADIAN(deltaTime), worldMatrix[1]);

		// can setup the attribs to be copied over once instead of each frame
		for (UINT i = 0; i < FSLogo_meshcount; i++)
		{
			MODEL_DATA md = { worldMatrix[i], FSLogo_materials[i].attrib };
			memcpy(&structuredBufferModelData[i], &md, sizeof(MODEL_DATA));
		}
	}

};
