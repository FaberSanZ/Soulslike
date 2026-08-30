#include "RenderingDevice.h"


namespace Engine
{
	class RenderSystem
	{
	public:
		RenderSystem() = default;

		void Initialize(HWND handle, uint32_t width, uint32_t height)
		{
			PresentationParameters presentation;
			presentation.handle = handle;
			presentation.width = width;
			presentation.height = height;


			// Initialize rendering system
			m_renderingDevice.Initialize();
			m_adapter = m_renderingDevice.CreateAdapter(0); // Select the first GPU adapter

			m_commandQueue = m_renderingDevice.CreateDirectQueue(m_adapter);
			m_swapChain = m_renderingDevice.CreateSwapChain(m_adapter, m_commandQueue, presentation);
		}

		void BeginFrame()
		{
			// Prepare for rendering
		}

		void Render()
		{
			// Render the scene
		}

		void EndFrame()
		{
			m_renderingDevice.Present(m_swapChain);
		}

		void Update(float deltaTime)
		{
			// Update rendering system
		}
		void Shutdown()
		{
			// Shutdown rendering system
		}

	private:
		RenderingDevice m_renderingDevice;
		Adapter m_adapter; // gpu selected adapter
		CommandQueue m_commandQueue; // command queue for submitting commands (default)
		SwapChain m_swapChain; // swap chain for presenting frames
	};


}
