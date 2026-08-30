// Engine.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "GameWindow.h"

int main()
{
	Engine::GameWindow window;
	window.Initialize();
	window.SetTitle(L"Engine Game Window");
	window.SetWindowSize(800, 600);

	while (window.IsRunning())
	{
		window.PumpMessages();

		if (Engine::GameInput::IsKeyPressed(Engine::GameInput::KeyCode::C))
			break;

	}
	window.Shutdown();
	

	return 0;
}

