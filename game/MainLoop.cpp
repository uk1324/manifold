#include "MainLoop.hpp"
#include <gfx/ShaderManager.hpp>

MainLoop::MainLoop()
	: visualization(Visualization::make()) {

}

void MainLoop::update() {
	ShaderManager::update();
	//game.update();
	visualization.update();
}