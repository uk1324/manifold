#pragma once

#include <engine/Math/Vec4.hpp>
#include <vector>

struct Polytope {
	// Storing just the sets of vertices making a 3-cell for example would only work if all of them were of the same type. For example if all of them were simplicies. This is why this needs to store the references to the one lower cells instead of vertices.
	using PointN = std::vector<f32>;
	using CellN = std::vector<i32>; // faces
	using CellsN = std::vector<CellN>;
	std::vector<PointN> vertices;
	std::vector<CellsN> cells;

	CellsN& cellsOfDimension(i32 n);
	const CellsN& cellsOfDimension(i32 n) const;
};

Polytope crossPolytope(i32 dimension);
i32 crossPolytopeSimplexCount(i32 dimensionOfCrossPolytope, i32 dimensionOfSimplex);

Polytope hypercube(i32 dimension);
i32 hypercubeCellCount(i32 dimensionOfHypercube, i32 dimensionOfCells);

// a face is a 2 cell.
std::vector<i32> faceVertices(const Polytope& p, i32 faceIndex);
std::vector<i32> faceVertices(const Polytope& p, const Polytope::CellN& face);