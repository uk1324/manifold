#pragma once

#include <game/Renderer.hpp>
#include <game/FpsCamera3d.hpp>
#include <game/SurfaceCamera.hpp>
#include <Array2d.hpp>
#include <game/PerlinNoise.hpp>

struct MainLoop {
	MainLoop();
	void update();
	void inSpaceUpdate(Vec3 cameraPosition);

	FpsCamera3d fpsCamera;
	SufaceCamera surfaceCamera;

	std::vector<Vec2> uvPositions;

	struct Surface {
		std::vector<Vec3> positions;
		std::vector<Vec3> normals;
		std::vector<Vec2> uvs;
		// These always go from 0 to 1, could compute then on the fly instead of precomputing.
		std::vector<Vec2> uvts;
		std::vector<i32> indices;

		std::vector<Vec3> triangleCenters;
		std::vector<f32> triangleAreas;
		f32 totalArea;

		Vec2 randomPosition();

		i32 vertexCount() const;
		i32 triangleCount() const;
		void addVertex(Vec3 p, Vec3 n, Vec2 uv, Vec2 uvt);
	};
	//std::vector<Vec2> randomPoints;

	std::random_device dev;
	std::default_random_engine rng;
	std::uniform_real_distribution<f32> uniform01;
	Vec2 randomPointOnSurface();

	Vec2 initialPositionUv = Vec2(0.1f);
	Vec2 initialVelocityUv = Vec2(0.0f, 1.0f);
	enum class Grabbed {
		NONE,
		POSITION,
		VELOCITY
	};
	Grabbed grabbed = Grabbed::NONE;

	Vec2 vectorInTangentSpaceBasis(Vec3 v, Vec3 tangentU, Vec3 tangentV, Vec3 normal) const;

	struct FlowParticles {
		std::vector<Vec2> positionsData;
		std::vector<Vec3> normalsData;
		std::vector<Vec3> colorsData;
		std::vector<Vec2> velocitiesData;
		Vec2& position(i32 particleIndex, i32 frame);
		Vec3& normal(i32 particleIndex, i32 frame);
		Vec3& color(i32 particleIndex, i32 frame);
		Vec2& velocity(i32 particleIndex, i32 frame);
		std::vector<i32> lifetime;
		std::vector<i32> elapsed;
		void initialize(i32 particleCount);
		i32 particleCount() const;
		void initializeParticle(i32 i, Vec2 position, Vec3 normal, i32 lifetime, Vec3 color, Vec2 velocity);
		static constexpr auto maxLifetime = 50;
	} flowParticles;
	void randomInitializeParticle(i32 i);

	PerlinNoise noise;
	void randomizeVectorField(usize seed);

	Vec3 vectorFieldSample(Vec3 v) const;

	f32 vectorFieldMinLength = 0.0f;
	f32 vectorFieldMaxLength = 1.0f;
	//Vec3 vectorFieldSample(Vec3 pos, Vec2 uvPos, Vec3 tangentU, Vec3 tangentV, Vec3 normal) const;

	Surface surfaceMesh;
	std::vector<i32> sortedTriangles;


	enum class CameraMode {
		ON_SURFACE,
		IN_SPACE
	} cameraMode = CameraMode::IN_SPACE;

	Renderer renderer;
};