#pragma once

#include <game/Game.hpp>

struct MainLoop {
	MainLoop();

	void update();

	Game game;
};