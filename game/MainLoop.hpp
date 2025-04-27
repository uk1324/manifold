#pragma once

#include <game/Game.hpp>
#include <game/Visualization.hpp>
#include <game/Visualization2.hpp>

struct MainLoop {
	MainLoop();

	void update();

	/*Game game;
	Visualization visualization;*/
	Visualization2 visualization2;
};