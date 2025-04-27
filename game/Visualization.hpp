#pragma once

#include <game/GameRenderer.hpp>
#include <game/PerlinNoise.hpp>
#include <game/FpsCamera3d.hpp>
#include <game/Polyhedra.hpp>

struct RandomSmoothRotationGenerator {
	RandomSmoothRotationGenerator();
	Vec3 positionOnSphere = Vec3(1.0f, 0.0f, 0.0f);
	Vec3 movementDirection = Vec3(0.0f, 1.0f, 0.0f);
	PerlinNoise noise;
	Quat positionOn3Sphere = Quat(1.0f, 1.0f, 0.0f, 0.0f).normalized();
	f32 rotationSpeed = 0.01f;
	f32 axisChangeChangeSpeed = 1.0f;
	f32 axisChangeSpeed = 1.0f;
	void settingsGui();

	void updateRotation();
	const Quat& getRotation() const;
};

struct Visualization {
	static Visualization make();

	void update();

	void renderPolygonSoup(const PolygonSoup& polygonSoup);

	bool rotateRandomly = false;
	RandomSmoothRotationGenerator randomRotationGenerator;

	bool drawStereographicProjection = false;

	enum class SymmetryGroup {
		IDENTITY,
		TETRAHEDRAL,
		OCTAHEDRAL,
		ICOSAHEDRAL,
	};
	SymmetryGroup symmetryGroup = SymmetryGroup::ICOSAHEDRAL;
	bool includeReflections = true;

	void sphereDrawing();

	void renderingSetup();
	void render();

	std::vector<std::vector<Vec3>> lines;
	std::optional<std::vector<Vec3>> currentlyDrawnLine;
	Vbo linesVbo;
	Ibo linesIbo;
	Vao linesVao;

	LineGenerator lineGenerator;

	GameRenderer renderer;
	FpsCamera3d camera;
};