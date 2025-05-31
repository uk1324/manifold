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

struct Game {
	static Game make();

	void update();

	std::vector<Vec4> vertices;
	std::vector<Vec3> verticesColors;
	
	struct Triangle {
		i32 vertices[3];
		Vec4 edgeNormals[3];
	};

	struct Edge {
		i32 vertices[2];
	};
	struct Face {
		std::vector<i32> vertices;
		std::vector<Vec4> edgeNormals;
		StaticList<i32, 2> cells;
		std::vector<Triangle> triangulation;
	};
	struct Cell {
		std::vector<i32> faces;
		std::vector<Vec4> faceNormals;
	};
	std::vector<Edge> edges;
	std::vector<Face> faces;
	std::vector<Cell> cells;

	std::vector<bool> isCellSet;

	enum class CameraType {
		NORMAL,
		STEREOGRAPHIC,
	} selectedCamera = CameraType::STEREOGRAPHIC;

	enum class Tool {
		BUILDING,
		PUSHING
	};
	Tool tool = Tool::BUILDING;

	World world;
	bool cellsModified = false;
	struct CellBody {
		i32 faceIndex;
		BodyId bodyId;
	};
	std::vector<CellBody> cellsBodies;
	void updateCellsBodies();

	GameRenderer renderer;
	FpsCamera3d camera;
	StereographicCamera stereographicCamera;
};