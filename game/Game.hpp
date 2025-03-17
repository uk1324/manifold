#pragma once

#include <game/FpsCamera3d.hpp>
#include <game/GameRenderer.hpp>
#include <game/SurfaceCamera.hpp>

struct Game {
	Game();

	void update();
	void renderOpaque();
	void renderTransparent(const Mat4& view);
	void render(const Mat4& view, Vec3 cameraPosition);

	enum class CameraMode {
		ON_SURFACE,
		IN_SPACE
	} cameraMode = CameraMode::ON_SURFACE;

	i32 frame = 0;

	f32 meshOpacity = 0.7f;
	//f32 meshOpacity = 1.0f;

	f32 bulletSize = 0.06f / 4.0f;
	f32 playerSize = 0.02f;
	f32 playerSpeed = 1.0f * 0.7f;
	f32 shiftPlayerSpeed = 0.5f * 0.7f;
	f32 cameraHeight = 0.4f;

	f32 randomAngle();

	GameRenderer renderer;

	enum class AttackType {
		MOVING_CIRCLES,
		CIRCLES,
	};
	std::optional<AttackType> currentAttackType;
	f32 elapsed = 0.0f;

	struct CirclesState {
		i32 frame;
		f32 elapsed;
	} circlesState;

	struct MovingCirclesState {
		i32 circleCount = 5;
		f32 time = 40.0f;
		//f32 holeSize = PI<f32> / 5.0f;
		f32 holeSize = PI<f32>;
		std::vector<f32> offsets;
	};
	MovingCirclesState movingCirclesState;

	struct ImmediateBullet {
		SurfacePosition position;
	};
	std::vector<ImmediateBullet> immediateBullets;

	struct PlayerBullet {
		SurfacePosition position;
		SurfaceTangent velocity;
		f32 lifetime;
	};
	std::vector<PlayerBullet> playerBullets;

	struct Bullet {
		SurfacePosition position;
		SurfaceTangent velocity;
		f32 lifetime;
	};
	std::vector<Bullet> bullets;

	struct BasicEnemy {
		SurfacePosition position;
		f32 hp;
	};
	std::vector<BasicEnemy> basicEnemies;

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