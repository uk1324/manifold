#pragma once

#include <game/FpsCamera3d.hpp>
#include <game/GameRenderer.hpp>
#include <game/SurfaceCamera.hpp>

struct Game {
	Game();

	void update();

	enum class CameraMode {
		ON_SURFACE,
		IN_SPACE
	} cameraMode = CameraMode::ON_SURFACE;

	GameRenderer renderer;

	FpsCamera3d fpsCamera;
	SufaceCamera surfaceCamera;
	Surface surface;
};