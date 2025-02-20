#pragma once

#include <game/Renderer.hpp>
#include <game/FpsCamera3d.hpp>

struct Visualization4d {
	void update(Renderer& renderer);

	FpsCamera3d fpsCamera;
};