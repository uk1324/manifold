#pragma once

#include <game/Polytopes.hpp>
#include <set>
#include <StaticList.hpp>

using CellIndex = i32;

struct Tiling {
	Tiling(const Polytope& polytope);

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
		Vec4 centroid;
		std::set<i32> vertices;
	};

	std::vector<Vec4> vertices;
	std::vector<Edge> edges;
	std::vector<Face> faces;
	std::vector<Cell> cells;

	std::vector<std::vector<i32>> cellsNeighbouringToCell();
};