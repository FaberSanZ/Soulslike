#include "GameWindow.h"
#include "RenderSystem.h"

using namespace Engine;

namespace Engine
{
	class GameBase
	{
	public:
		GameBase() = default;

		void Run()
		{
			m_window.Initialize();
			m_window.SetTitle(L"Engine Game Window");
			m_window.SetWindowSize(800, 600);

			OnInitialize();
			
			m_renderSystem.Initialize(m_window.Handle(), m_window.ClientWidth(), m_window.ClientHeight(), m_shadersSystem);

			while (m_window.IsRunning())
			{
				m_window.PumpMessages();
				OnUpdate(0);
				OnFixedUpdate();

				m_renderSystem.BeginFrame();
				m_renderSystem.Render();
				m_renderSystem.EndFrame();
			}

			OnShutdown();
			m_window.Shutdown();
			m_renderSystem.Shutdown();
		}

	protected:
		virtual void OnInitialize() = 0;
		virtual void OnUpdate(float deltaTime) = 0;
		virtual void OnFixedUpdate() = 0;
		virtual void OnShutdown() = 0;

		GameWindow& GetWindow() { return m_window; }

	private:
		GameWindow m_window;
		RenderSystem m_renderSystem;
		ShadersSystem m_shadersSystem;

	};

}
