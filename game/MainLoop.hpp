#pragma once

#include <game/Game.hpp>
#include <game/Minesweeper.hpp>

struct MainLoop {
	MainLoop();

	void update();

	//Game game;
	Minesweeper minesweeper;
	GameRenderer renderer;
};