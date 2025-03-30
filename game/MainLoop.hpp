#pragma once

#include <game/Game.hpp>
#include <game/Visualization.hpp>

struct MainLoop {
	MainLoop();

	void update();

	Game game;
	Visualization visualization;
};