#pragma once

#include <cstdint>
#include <vector>
#include <dxgi1_4.h>
#include <d3d12.h>

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

		void Present(SwapChain& swapChain, uint32_t syncInterval = 1, uint32_t flags = 0)
		{
			swapChain.m_swapChain->Present(syncInterval, flags);
			swapChain.m_backBufferIndex = swapChain.m_swapChain->GetCurrentBackBufferIndex();
		}
	};
}
