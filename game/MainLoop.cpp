#include "MainLoop.hpp"
#include <gfx/ShaderManager.hpp>

void MainLoop::update() {
	ShaderManager::update();
	game.update();
}