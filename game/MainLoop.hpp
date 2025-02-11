#pragma once

#include <game/Renderer.hpp>

struct MainLoop {
	MainLoop();
	void update();

	Renderer renderer;
};