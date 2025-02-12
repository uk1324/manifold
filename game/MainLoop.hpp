#pragma once

#include <game/Renderer.hpp>

struct MainLoop {
	MainLoop();
	void update();

	Vec2 uvPosition = Vec2(0.1f);
	f32 uvForwardAngle = 0.0f;
	f32 rightAxisAngle = 0.0f;
	std::optional<Vec2> lastMousePosition;
	std::vector<Vec2> uvPositions;


	Renderer renderer;
};