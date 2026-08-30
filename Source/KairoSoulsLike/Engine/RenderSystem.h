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
	};


}
