#include <cstdint>
#include <dxgi1_4.h>

#pragma comment(lib, "dxgi.lib")	


namespace Engine
{
	class RenderingDevice;
	class Adapter
	{
		private:
			friend class RenderingDevice;

			IDXGIFactory4* m_factory;
			IDXGIAdapter1* m_adapter = nullptr;

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

			return adapter;
		}
	private:

	};

}
