#pragma once

#include <cstdint>
#include <cstring>
#include <vector>
#include <dxgi1_4.h>
#include <d3d12.h>
#include "ShadersSystem.h"

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")

namespace Engine
{
	class RenderingDevice;

	class Adapter
	{
	private:
		friend class RenderingDevice;

		IDXGIFactory4* m_factory = nullptr;
		IDXGIAdapter1* m_adapter = nullptr;
		ID3D12Device* m_device = nullptr;
	};

	struct PresentationParameters
	{
		HWND handle = nullptr;
		uint32_t width = 1280;
		uint32_t height = 720;
		uint32_t bufferCount{ 2 };
		DXGI_FORMAT format{ DXGI_FORMAT_R8G8B8A8_UNORM };
	};

	class DescriptorHeap
	{
	private:
		friend class RenderingDevice;

		ID3D12DescriptorHeap* m_Heap = nullptr;
		uint32_t m_DescriptorSize = 0;
	};

	class Texture
	{
	private:
		friend class RenderingDevice;

		ID3D12Resource* m_resource = nullptr;
		D3D12_CPU_DESCRIPTOR_HANDLE m_rtv{};
	};

	class CommandQueue
	{
	private:
		friend class RenderingDevice;

		ID3D12CommandQueue* m_queue = nullptr;
	};

	class SwapChain
	{
	public:
		Texture& BackBuffer()
		{
			return m_backBuffers[m_backBufferIndex];
		}

	private:
		friend class RenderingDevice;

		IDXGISwapChain3* m_swapChain = nullptr;
		uint32_t m_backBufferIndex = 0;
		std::vector<Texture> m_backBuffers;
		DescriptorHeap m_rtvHeap;
	};

	class PipelineState
	{
	private:
		friend class RenderingDevice;

		ID3D12RootSignature* m_rootSignature = nullptr;
		ID3D12PipelineState* m_pipelineState = nullptr;
	};

	class CommandList
	{
	private:
		friend class RenderingDevice;

		ID3D12CommandAllocator* m_commandAllocator = nullptr;
		ID3D12GraphicsCommandList* m_commandList = nullptr;
	};

	class Buffer
	{
	private:
		friend class RenderingDevice;

		ID3D12Resource* m_resource = nullptr;
		uint32_t m_size = 0;
	};

	struct MeshPart
	{
		Buffer m_vertexBuffer;
		Buffer m_indexBuffer;

		D3D12_INDEX_BUFFER_VIEW m_indexBufferView{};

		uint32_t m_vertexCount = 0;
		uint32_t m_indexCount = 0;
	};

	class RenderingDevice
	{
	public:
		RenderingDevice() = default;

		void Initialize()
		{
			// Initialize the rendering device
		}

		void Shutdown()
		{
			// Shutdown the rendering device
		}

		Adapter CreateAdapter(uint32_t idx_gpu)
		{
			Adapter adapter;

			CreateDXGIFactory1(IID_PPV_ARGS(&adapter.m_factory));
			adapter.m_factory->EnumAdapters1(idx_gpu, &adapter.m_adapter);
			D3D12CreateDevice(adapter.m_adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&adapter.m_device));

			return adapter;
		}

		CommandQueue CreateDirectQueue(Adapter& adapter)
		{
			CommandQueue commandQueue;

			D3D12_COMMAND_QUEUE_DESC queueDesc{};
			queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

			adapter.m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue.m_queue));

			return commandQueue;
		}

		SwapChain CreateSwapChain(Adapter& adapter, CommandQueue& commandQueue, const PresentationParameters& params)
		{
			SwapChain swapChain;

			DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
			swapChainDesc.Width = params.width;
			swapChainDesc.Height = params.height;
			swapChainDesc.Format = params.format;
			swapChainDesc.SampleDesc.Count = 1;
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.BufferCount = params.bufferCount;
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

			IDXGISwapChain1* tempSwapChain = nullptr;

			adapter.m_factory->CreateSwapChainForHwnd(commandQueue.m_queue, params.handle, &swapChainDesc, nullptr, nullptr, &tempSwapChain);
			tempSwapChain->QueryInterface(IID_PPV_ARGS(&swapChain.m_swapChain));
			tempSwapChain->Release();

			swapChain.m_backBufferIndex = swapChain.m_swapChain->GetCurrentBackBufferIndex();
			swapChain.m_backBuffers.resize(params.bufferCount);
			swapChain.m_rtvHeap = InitializeDescriptorHeap(adapter, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, params.bufferCount);

			D3D12_CPU_DESCRIPTOR_HANDLE rtv = swapChain.m_rtvHeap.m_Heap->GetCPUDescriptorHandleForHeapStart();

			for (uint32_t i = 0; i < params.bufferCount; i++)
			{
				Texture& texture = swapChain.m_backBuffers[i];

				swapChain.m_swapChain->GetBuffer(i, IID_PPV_ARGS(&texture.m_resource));
				texture.m_rtv = rtv;

				adapter.m_device->CreateRenderTargetView(texture.m_resource, nullptr, rtv);

				rtv.ptr += swapChain.m_rtvHeap.m_DescriptorSize;
			}

			return swapChain;
		}

		PipelineState CreatePipelineState(Adapter& adapter, ShadersSystem& shaders)
		{
			PipelineState pipelineState;

			IDxcBlob* vertexShader = shaders.Compile(L"Assets/Shaders/Vertex.hlsl", L"VS", L"vs_6_0");
			IDxcBlob* pixelShader = shaders.Compile(L"Assets/Shaders/Pixel.hlsl", L"PS", L"ps_6_0");

			D3D12_ROOT_PARAMETER vertexBufferParameter{};
			vertexBufferParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
			vertexBufferParameter.Descriptor.ShaderRegister = 0;
			vertexBufferParameter.Descriptor.RegisterSpace = 0;
			vertexBufferParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

			D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
			rootSignatureDesc.NumParameters = 1;
			rootSignatureDesc.pParameters = &vertexBufferParameter;

			ID3DBlob* rootSignatureBlob = nullptr;
			ID3DBlob* errorBlob = nullptr;

			D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rootSignatureBlob, &errorBlob);
			adapter.m_device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(), rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&pipelineState.m_rootSignature));

			D3D12_RASTERIZER_DESC rasterizerDesc{};
			rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
			rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
			rasterizerDesc.FrontCounterClockwise = FALSE;
			rasterizerDesc.DepthClipEnable = TRUE;

			D3D12_BLEND_DESC blendDesc{};
			blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

			D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc{};
			pipelineDesc.pRootSignature = pipelineState.m_rootSignature;
			pipelineDesc.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
			pipelineDesc.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };
			pipelineDesc.BlendState = blendDesc;
			pipelineDesc.RasterizerState = rasterizerDesc;
			pipelineDesc.DepthStencilState.DepthEnable = FALSE;
			pipelineDesc.DepthStencilState.StencilEnable = FALSE;
			pipelineDesc.SampleMask = UINT_MAX;
			pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			pipelineDesc.NumRenderTargets = 1;
			pipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			pipelineDesc.SampleDesc.Count = 1;

			adapter.m_device->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(&pipelineState.m_pipelineState));

			return pipelineState;
		}

		DescriptorHeap InitializeDescriptorHeap(Adapter& adapter, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT numDescriptors)
		{
			DescriptorHeap heap;

			D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
			heapDesc.Type = type;
			heapDesc.NumDescriptors = numDescriptors;
			heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			heapDesc.NodeMask = 0;

			adapter.m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heap.m_Heap));
			heap.m_DescriptorSize = adapter.m_device->GetDescriptorHandleIncrementSize(type);

			return heap;
		}

		CommandList CreateCommandList(Adapter& adapter)
		{
			CommandList commandList;

			adapter.m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandList.m_commandAllocator));
			adapter.m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandList.m_commandAllocator, nullptr, IID_PPV_ARGS(&commandList.m_commandList));
			commandList.m_commandList->Close();

			return commandList;
		}

		Buffer CreateBuffer(Adapter& adapter, const void* data, uint32_t size)
		{
			Buffer buffer;

			D3D12_HEAP_PROPERTIES heapProperties{};
			heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

			D3D12_RESOURCE_DESC bufferDesc{};
			bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			bufferDesc.Width = size;
			bufferDesc.Height = 1;
			bufferDesc.DepthOrArraySize = 1;
			bufferDesc.MipLevels = 1;
			bufferDesc.SampleDesc.Count = 1;
			bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

			adapter.m_device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&buffer.m_resource));

			void* mappedData = nullptr;

			buffer.m_resource->Map(0, nullptr, &mappedData);
			memcpy(mappedData, data, size);
			buffer.m_resource->Unmap(0, nullptr);
			buffer.m_size = size;

			return buffer;
		}

		MeshPart CreateMeshPart(Adapter& adapter, const void* vertexData, uint32_t vertexSize, uint32_t vertexCount, const void* indexData, uint32_t indexSize, uint32_t indexCount)
		{
			MeshPart meshPart;

			meshPart.m_vertexBuffer = CreateBuffer(adapter, vertexData, vertexSize);
			meshPart.m_indexBuffer = CreateBuffer(adapter, indexData, indexSize);

			meshPart.m_vertexCount = vertexCount;
			meshPart.m_indexCount = indexCount;

			meshPart.m_indexBufferView.BufferLocation = meshPart.m_indexBuffer.m_resource->GetGPUVirtualAddress();
			meshPart.m_indexBufferView.SizeInBytes = indexSize;
			meshPart.m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;

			return meshPart;
		}

		void DrawMeshPart(CommandList& commandList, MeshPart& meshPart)
		{
			commandList.m_commandList->SetGraphicsRootShaderResourceView(0, meshPart.m_vertexBuffer.m_resource->GetGPUVirtualAddress());
			commandList.m_commandList->IASetIndexBuffer(&meshPart.m_indexBufferView);
			commandList.m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			commandList.m_commandList->DrawIndexedInstanced(meshPart.m_indexCount, 1, 0, 0, 0);
		}

		void BeginFrame(CommandList& commandList)
		{
			commandList.m_commandAllocator->Reset();
			commandList.m_commandList->Reset(commandList.m_commandAllocator, nullptr);
		}

		void Clear(CommandList& commandList, Texture& renderTarget, const float clearColor[4])
		{
			commandList.m_commandList->ClearRenderTargetView(renderTarget.m_rtv, clearColor, 0, nullptr);
		}

		void Execute(CommandQueue& commandQueue, CommandList& commandList)
		{
			ID3D12CommandList* listsToExecute[] = { commandList.m_commandList };
			commandQueue.m_queue->ExecuteCommandLists(1, listsToExecute);
		}

		void EndFrame(CommandList& commandList)
		{
			commandList.m_commandList->Close();
		}

		void Present(SwapChain& swapChain, uint32_t syncInterval = 1, uint32_t flags = 0)
		{
			swapChain.m_swapChain->Present(syncInterval, flags);
			swapChain.m_backBufferIndex = swapChain.m_swapChain->GetCurrentBackBufferIndex();
		}

		void SetRenderTarget(CommandList& commandList, Texture& renderTarget)
		{
			commandList.m_commandList->OMSetRenderTargets(1, &renderTarget.m_rtv, FALSE, nullptr);
		}

		void SetViewport(CommandList& commandList, uint32_t width, uint32_t height)
		{
			D3D12_VIEWPORT viewport{ 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f };
			D3D12_RECT scissor{ 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };

			commandList.m_commandList->RSSetViewports(1, &viewport);
			commandList.m_commandList->RSSetScissorRects(1, &scissor);
		}

		void SetPipelineState(CommandList& commandList, PipelineState& pipelineState)
		{
			commandList.m_commandList->SetPipelineState(pipelineState.m_pipelineState);
			commandList.m_commandList->SetGraphicsRootSignature(pipelineState.m_rootSignature);
		}

		void SetVertexBuffer(CommandList& commandList, Buffer& buffer)
		{
			commandList.m_commandList->SetGraphicsRootShaderResourceView(0, buffer.m_resource->GetGPUVirtualAddress());
		}

		void Draw(CommandList& commandList, uint32_t vertexCount)
		{
			commandList.m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			commandList.m_commandList->DrawInstanced(vertexCount, 1, 0, 0);
		}
	};
}
