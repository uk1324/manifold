#pragma once

#include <game/GameRenderer.hpp>
#include <game/PerlinNoise.hpp>
#include <game/FpsCamera3d.hpp>
#include <game/StereographicCamera.hpp>
#include <game/Polytopes.hpp>
#include <game/Polyhedra.hpp>
#include <engine/Math/Vec4.hpp>
#include <StaticList.hpp>
#include <game/Physics/World.hpp>

struct PolytopeEdge {
	i32 vertices[2];
};

struct PolytopeFace {
	std::vector<i32> edges;
};

struct PolytopeCell3 {
	std::vector<i32> faces;
};

struct Polytope4 {
	Polytope4(const Polytope& p);

	std::vector<Vec4> vertices;
	std::vector<PolytopeEdge> edges;
	std::vector<PolytopeFace> faces;
	std::vector<PolytopeCell3> cells;
};

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
		std::vector<Vec4> edgeNormals;
		StaticList<i32, 2> cells;
	};
	struct Cell {
		std::vector<i32> faces;
		std::vector<Vec4> faceNormals;
	};
	std::vector<Edge> edges;
	std::vector<Face> faces;
	std::vector<Cell> cells;

	std::vector<bool> isCellSet;

	Vbo linesVbo;
	Ibo linesIbo;
	Vao linesVao;
	//Polytope crossPolytope;
	enum class CameraType {
		NORMAL,
		STEREOGRAPHIC,
	} selectedCamera = CameraType::STEREOGRAPHIC;

	enum class Tool {
		BUILDING,
		PUSHING
	};
	Tool tool = Tool::BUILDING;

	LineGenerator lineGenerator;

	std::vector<GameRenderer::SphereLodSetting> lodLevelsSettings;
	void lodLevelsSettingsGui();

	World world;

	GameRenderer renderer;
	FpsCamera3d camera;
	StereographicCamera stereographicCamera;
};