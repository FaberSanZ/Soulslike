#include "RenderingDevice.h"


namespace Engine
{
	class RenderSystem
	{
	public:
		RenderSystem() = default;

		void Initialize()
		{
			// Initialize rendering system
			m_renderingDevice.Initialize();
			m_adapter = m_renderingDevice.CreateAdapter(0); // Select the first GPU adapter
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
			// Finalize rendering
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
	};


}
