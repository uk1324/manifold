#pragma once

#include <game/FpsCamera3d.hpp>
#include <game/GameRenderer.hpp>
#include <game/EntityArray.hpp>
#include <game/SurfaceCamera.hpp>
#include <engine/Math/Color.hpp>
#include <Array2d.hpp>

template<typename T>
struct LeaveUninitialized {
	T operator()() { return T(); };
};

struct PlayerBullet {
	SurfacePosition position;
	SurfaceTangent velocity;
	f32 lifetime;
};
struct Enemy {
	SurfacePosition position;
	f32 hp;
};
using EnemyId = EntityArrayId<Enemy>;
struct Bullet {
	SurfacePosition position;
	SurfaceTangent velocity;
	Vec3 color;
	f32 lifetime;
};
using BulletId = EntityArrayId<Bullet>;


struct BulletCollection {
	f32 lifetime;
	SurfacePosition position;
	SurfaceTangent velocity;
	std::vector<BulletId> bullets;
};

struct Game;

struct Wave {
	virtual bool update(Game& c) = 0;
};

struct RandomEnemiesShootingWave : Wave {
	void initialize();
	bool update(Game& c);

	std::vector<EnemyId> spawnedEnemies;
	i32 frame;
};

struct SpiralEnemyWave : Wave {
	void initialize(Game& c);
	bool update(Game& c);
	EnemyId enemy = EnemyId::invalid();
	enum class ShotType {
		SPIRAL,
		DIPOLE,
		WALLS,
		CIRCLE_COLLECTIONS
	};
	ShotType shotType = ShotType::CIRCLE_COLLECTIONS;
	f32 angle;
	i32 frame;
};

struct CircleSpawning : Wave {
	void initialize(Game& c);
	bool update(Game& c);
	f32 u;
	f32 v;
	i32 frame;
	std::vector<EnemyId> enemies;
	i32 enemyCount;
};

struct SpawningAroundThePlayer : Wave {
	void initialize(Game& c);
	bool update(Game& c);
	i32 frame;
};

struct Game {
	Game();

	Array2d<Vec3> surfacePositions;
	std::vector<f32> neigbourDistances;

	void update();
	void renderOpaque();
	void renderTransparent(const Mat4& view);
	void render(const Mat4& view, Vec3 cameraPosition);

	enum class CameraMode {
		ON_SURFACE,
		IN_SPACE
	} cameraMode = CameraMode::ON_SURFACE;

	i32 frame = 0;

	Vec3 playerPosition3;

	std::optional<Wave*> currentWave;

	EntityArrayPair<Bullet> spawnBullet(SurfacePosition position, SurfaceTangent velocity, f32 lifetime, Vec3 color = Color3::RED);
	void bulletCircle(SurfacePosition position, f32 initialAngle, i32 angleCount, f32 velocity, f32 lifetime, Vec3 color = Color3::RED);
	EntityArrayPair<Enemy> spawnEnemy(SurfacePosition position, f32 hp);
	EntityArrayPair<BulletCollection> bulletCollection(SurfacePosition position, SurfaceTangent velocity, f32 lifetime, Vec3 color = Color3::RED);

	RandomEnemiesShootingWave randomEnemiesShootingWave;
	SpiralEnemyWave spiralWave;
	CircleSpawning circleSpawningWave;
	SpawningAroundThePlayer spawnAroundPlayerWave;

	std::vector<PlayerBullet> playerBullets;
	EntityArray<Enemy, LeaveUninitialized<Enemy>> enemies;
	EntityArray<Bullet, LeaveUninitialized<Bullet>> bullets;
	EntityArray<BulletCollection, LeaveUninitialized<BulletCollection>> bulletCollections;

	SurfaceTangent tryAimAtPlayer(SurfacePosition sourcePosition, f32 velocity);
	f32 tryAimAtPlayerAngle(SurfacePosition sourcePosition);

	f32 meshOpacity = 0.7f;
	//f32 meshOpacity = 1.0f;

	f32 bulletSize = 0.06f / 4.0f;
	f32 playerSize = 0.02f;
	f32 playerBulletSize = 0.01f;
	f32 playerSpeed = 1.0f * 0.7f;
	f32 shiftPlayerSpeed = 0.5f * 0.7f;
	f32 cameraHeight = 0.4f;

	bool wireframe = false;

	f32 random01();
	f32 randomMinus1To1();
	f32 randomAngle();

	GameRenderer renderer;

	FpsCamera3d fpsCamera;
	SufaceCamera surfaceCamera;
	Surface surface;
};