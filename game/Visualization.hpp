#pragma once

#include <game/GameRenderer.hpp>
#include <game/PerlinNoise.hpp>
#include <game/FpsCamera3d.hpp>
#include <game/Polyhedra.hpp>

struct Visualization {
	Visualization();

	void update();

	void renderPolygonSoup(const PolygonSoup& polygonSoup);

	// Don't think it is possible to continously assign angles on a sphere, then you could create a non-vanishing vector field for example by getting all the tangent vectors with angle 0.
	std::vector<Vec3> positions;
	Vec3 positionOnSphere = Vec3(1.0f, 0.0f, 0.0f);
	Vec3 movementDirection = Vec3(0.0f, 1.0f, 0.0f);
	PerlinNoise noise;
	Quat positionOn3Sphere = Quat(1.0f, 1.0f, 0.0f, 0.0f).normalized();
	f32 rotationSpeed = 1.0f;
	f32 axisChangeSpeed1 = 1.0f;
	f32 axisChangeSpeed2 = 1.0f;

	GameRenderer renderer;
	FpsCamera3d camera;
};