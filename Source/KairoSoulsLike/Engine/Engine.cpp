// Engine.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "GameWindow.h"
#include "GameBase.h"

using namespace Engine;

class MyGame final : public GameBase
{
protected:
	void OnInitialize() override
	{

	}

	void OnUpdate(float deltaTime) override
	{

	}

	void OnFixedUpdate() override
	{
	}

	void OnShutdown() override
	{

	}
};

int main()
{
	MyGame game;
	game.Run();

	return 0;
}

