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

	f32 bulletSize = 0.06f;
	f32 playerSize = 0.03f;
	f32 playerSpeed = 1.0f * 0.7f;
	f32 shiftPlayerSpeed = 0.5f * 0.7f;
	f32 cameraHeight = 0.4f;

	GameRenderer renderer;

	struct Bullet {
		SurfacePosition position;
		SurfaceTangent velocity;
		f32 lifetime;
	};
	std::vector<Bullet> bullets;

	struct SpiralEmitter {
		f32 lifetime;
		f32 angle = 0.0f;
		i32 frame = 0;
		SurfacePosition position;
	};
	std::vector<SpiralEmitter> spiralEmitters;

	struct DirectionalEmitter {
		f32 lifetime;
		SurfaceTangent direction;
		SurfacePosition position;
		i32 frame;
	};
	std::vector<DirectionalEmitter> directionalEmitters;

	struct BasicEmitter {
		f32 elapsed;
		SurfacePosition position;
	};
	std::vector<BasicEmitter> basicEmitters;

	FpsCamera3d fpsCamera;
	SufaceCamera surfaceCamera;
	Surface surface;
};