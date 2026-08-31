#include "RenderingDevice.h"


namespace Engine
{
	struct VertexPositionColor
	{
		float position[4];
		float color[4];
	};


	class RenderSystem
	{
	public:
		RenderSystem() = default;

		void Initialize(HWND handle, uint32_t width, uint32_t height, ShadersSystem& shaders)
		{

			m_width = width;
			m_height = height;

			PresentationParameters presentation;
			presentation.handle = handle;
			presentation.width = width;
			presentation.height = height;


			// Initialize rendering system
			m_renderingDevice.Initialize();
			m_adapter = m_renderingDevice.CreateAdapter(0); // Select the first GPU adapter
			m_commandQueue = m_renderingDevice.CreateDirectQueue(m_adapter);
			m_swapChain = m_renderingDevice.CreateSwapChain(m_adapter, m_commandQueue, presentation);
			m_commandList = m_renderingDevice.CreateCommandList(m_adapter);
			m_pipelineState = m_renderingDevice.CreatePipelineState(m_adapter, shaders);



			struct Vertex
			{
				float position[4];
				float color[4];
			};

			Vertex vertices[] =
			{
				{ { -0.5f,  0.5f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
				{ {  0.5f,  0.5f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
				{ {  0.5f, -0.5f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
				{ { -0.5f, -0.5f, 0.0f, 1.0f }, { 1.0f, 0.0f, 1.0f, 1.0f } }
			};

			uint32_t indices[] =
			{
				0, 1, 2,
				0, 2, 3
			};

			m_meshPart = m_renderingDevice.CreateMeshPart(m_adapter, vertices, sizeof(vertices), 4, indices, sizeof(indices), 6);

		}

		void BeginFrame()
		{
			m_renderingDevice.BeginFrame(m_commandList);
		}

		void Render()
		{
			const float clearColor[4] = { 0.0f, 0.2f, 0.4f, 1.0f };

			Texture& backBuffer = m_swapChain.BackBuffer();

			m_renderingDevice.SetRenderTarget(m_commandList, backBuffer);
			m_renderingDevice.Clear(m_commandList, backBuffer, clearColor);
			m_renderingDevice.SetViewport(m_commandList, m_width, m_height);
			m_renderingDevice.SetPipelineState(m_commandList, m_pipelineState);
			m_renderingDevice.DrawMeshPart(m_commandList, m_meshPart);
		}

		void EndFrame()
		{
			m_renderingDevice.EndFrame(m_commandList);
			m_renderingDevice.Execute(m_commandQueue, m_commandList);
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
		CommandList m_commandList; // command list for recording commands
		PipelineState m_pipelineState; // pipeline state for rendering

		uint32_t m_width = 0;
		uint32_t m_height = 0;

		MeshPart m_meshPart;
	};


}
