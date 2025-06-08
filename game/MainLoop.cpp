#include "MainLoop.hpp"
#include <gfx/ShaderManager.hpp>

MainLoop::MainLoop()
	: game(Game::make())
	, renderer(GameRenderer::make())
{

}

void MainLoop::update() {
	ShaderManager::update();
	//game.update(renderer);
	minesweeper.update(renderer);
}