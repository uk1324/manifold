#pragma once

#include <game/GameRenderer.hpp>
#include <game/PerlinNoise.hpp>
#include <game/FpsCamera3d.hpp>
#include <game/StereographicCamera.hpp>
#include <game/Polytopes.hpp>
#include <game/Polyhedra.hpp>
#include <engine/Math/Vec4.hpp>

struct Visualization2 {
	static Visualization2 make();

	void update();

	std::vector<Vec4> vertices;
	std::vector<Vec3> verticesColors;
	struct Edge {
		i32 vertices[2];
	};
	struct Face {
		std::vector<i32> vertices;
	};
	struct Cell {
		std::vector<i32> faces;
	};
	std::vector<Edge> edges;
	std::vector<Face> faces;
	std::vector<Cell> cells;

	Vbo linesVbo;
	Ibo linesIbo;
	Vao linesVao;
	//Polytope crossPolytope;
	enum class CameraType {
		NORMAL,
		STEREOGRAPHIC,
	} selectedCamera = CameraType::STEREOGRAPHIC;

	LineGenerator lineGenerator;

	GameRenderer renderer;
	FpsCamera3d camera;
	StereographicCamera stereographicCamera;
};