#include "MainLoop.hpp"
#include <gfx/ShaderManager.hpp>

MainLoop::MainLoop()
	: game(Game::make())
{

}

void MainLoop::update() {
	ShaderManager::update();
	game.update();
}