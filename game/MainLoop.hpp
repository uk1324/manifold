#pragma once

#include <game/Renderer.hpp>
#include <game/FpsCamera3d.hpp>
#include <game/SurfaceCamera.hpp>

struct MainLoop {
	MainLoop();
	void update();

	FpsCamera3d fpsCamera;
	SufaceCamera surfaceCamera;

	std::vector<Vec2> uvPositions;

	struct Surface {
		std::vector<Vec3> positions;
		std::vector<Vec3> normals;
		std::vector<Vec2> uvs;
		std::vector<i32> indices;
		i32 vertexCount() const;
		i32 triangleCount() const;
		void addVertex(Vec3 p, Vec3 n, Vec2 uv);
	};

	Surface surfaceMesh;

	enum class CameraMode {
		ON_SURFACE,
		IN_SPACE
	} cameraMode = CameraMode::IN_SPACE;

	Renderer renderer;
};