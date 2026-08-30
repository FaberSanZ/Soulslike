#include <cstdint>

namespace Engine
{
	class RenderingDevice;

	class DescriptorHeap
	{
	private:
		friend class RenderingDevice;
		uint32_t m_capacity;
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
	};

}
