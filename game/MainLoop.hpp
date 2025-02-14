#pragma once

#include <game/Renderer.hpp>
#include <game/FpsCamera3d.hpp>
#include <game/SurfaceCamera.hpp>

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
		i32 vertexCount() const;
		i32 triangleCount() const;
		void addVertex(Vec3 p, Vec3 n, Vec2 uv, Vec2 uvt);
	};

	Vec2 initialPositionUv = Vec2(0.1f);
	Vec2 initialVelocityUv = Vec2(0.0f, 1.0f);
	enum class Grabbed {
		NONE,
		POSITION,
		VELOCITY
	};
	Grabbed grabbed = Grabbed::NONE;

	Surface surfaceMesh;
	std::vector<i32> sortedTriangles;

	enum class CameraMode {
		ON_SURFACE,
		IN_SPACE
	} cameraMode = CameraMode::IN_SPACE;

	Renderer renderer;
};