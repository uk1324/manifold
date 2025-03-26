#pragma once

#include <game/Game.hpp>
#include <game/Visualization.hpp>

struct MainLoop {
	void update();

	Game game;
	Visualization visualization;
};