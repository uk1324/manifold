#include "MainLoop.hpp"
#include <gfx/ShaderManager.hpp>

MainLoop::MainLoop()
	//: visualization(Visualization::make())
	: visualization2(Visualization2::make())
{

}

void MainLoop::update() {
	ShaderManager::update();
	//game.update();
	//visualization.update();
	visualization2.update();
}