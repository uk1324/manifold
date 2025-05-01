#include "Polytopes.hpp"
#include "Combinatorics.hpp"

void addPyramid(
	Polytope& polytope, 
	i32 pyramidVertexIndex, 
	// This is used for example for making a bipyramid. The procedure shouldn't connect the vertices added in the first call to the pyramid vertex so this only iterates for the ones with indices lower than the bound.
	const std::vector<i32>& upperIndexBoundsForBase) {
	const auto baseDimension = upperIndexBoundsForBase.size();

	// The 1-cell coming out of vertex with index i will have index newCellsOffset + i.
	i32 newCellsOffset = polytope.cellsOfDimension(1).size();
	for (i32 vertexI = 0; vertexI < upperIndexBoundsForBase[0]; vertexI++) {
		auto& cells = polytope.cellsOfDimension(1);
		cells.push_back(Polytope::CellN{ vertexI, pyramidVertexIndex });
	}
	for (i32 dimension = 2; dimension < baseDimension + 1; dimension++) {
		auto& dimensionCells = polytope.cellsOfDimension(dimension);
		const auto cellOffset = polytope.cellsOfDimension(dimension).size();

		auto& baseCells = polytope.cellsOfDimension(dimension - 1);
		for (i32 baseCellI = 0; baseCellI < upperIndexBoundsForBase[dimension - 1]; baseCellI++) {
			auto& baseCell = baseCells[baseCellI];
			Polytope::CellN newCell;
			newCell.push_back(baseCellI);
			for (const auto& cell : baseCell) {
				newCell.push_back(newCellsOffset + cell);
			}
			dimensionCells.push_back(newCell);
		}
		newCellsOffset = cellOffset;
	}
}

Polytope crossPolytope(i32 dimension) {
	ASSERT(dimension >= 2);

	if (dimension == 2) {
		Polytope result;
		result.vertices.push_back({ 1.0f, 0.0f }); // 0
		result.vertices.push_back({ 0.0f, 1.0f }); // 1
		result.vertices.push_back({ -1.0f, 0.0f }); // 2
		result.vertices.push_back({ 0.0f, -1.0f }); // 3
		result.cells.push_back(Polytope::CellsN{});
		auto& cells1 = result.cells[0];
		cells1.push_back({ 0, 1 });
		cells1.push_back({ 1, 2 });
		cells1.push_back({ 2, 3 });
		cells1.push_back({ 3, 0 });
		return result;
	}

	auto result = crossPolytope(dimension - 1);
	for (auto& vertex : result.vertices) {
		vertex.push_back(0.0f);
	}
	Polytope::PointN newVertexStart;
	for (i32 i = 0; i < dimension - 1; i++) {
		newVertexStart.push_back(0.0f);
	}
	const f32 directions[] = { 1.0f, -1.0f };
	for (const auto& direction : directions) {
		auto copy = Polytope::PointN(newVertexStart);
		copy.push_back(direction);
		result.vertices.push_back(std::move(copy));
	}
	const auto apex0 = result.vertices.size() - 2;
	const auto apex1 = result.vertices.size() - 1;
	
	std::vector<i32> upperIndexBoundsForBase;
	upperIndexBoundsForBase.push_back(result.vertices.size() - 2);
	for (const auto& cells : result.cells) {
		upperIndexBoundsForBase.push_back(cells.size());
	}
	result.cells.push_back(Polytope::CellsN());
	addPyramid(result, apex0, upperIndexBoundsForBase);
	addPyramid(result, apex1, upperIndexBoundsForBase);
	return result;
}

template<typename T>
T integerToNonNegativePower(T base, T power) {
	ASSERT(power >= 0);
	if (power == 0) {
		return 1;
	}

	if (power % 2 == 0) {
		const auto v = integerToNonNegativePower(base, power / 2);
		return v * v;
	} else {
		return base * integerToNonNegativePower(base, power - 1);
	}
}

i32 crossPolytopeSimplexCount(i32 dimensionOfCrossPolytope, i32 dimensionOfSimplex) {
	const auto& n = dimensionOfCrossPolytope;
	const auto& k = dimensionOfSimplex;
	// Difference equation
	// S(n, k) = S(n-1, k) + 2 * S(n-1, k-1)
	// S(n, 0) = 2 * n
	// Solution:
	// S(n, k) = 2^(k+1) (n choose k + 1)
	return integerToNonNegativePower(2, k + 1) * nChoosek(n, k + 1);
}

std::vector<i32> faceVertices(const Polytope& p, i32 faceIndex) {
	return faceVertices(p, p.cellsOfDimension(2)[faceIndex]);
}

std::vector<i32> faceVertices(const Polytope& p, const Polytope::CellN& face) {
	const auto& faceEdgesIdxs = face;
	const auto& edges = p.cellsOfDimension(1);

	i32 startIndex = 0;
	std::vector<i32> vertices;

	vertices.push_back(edges[faceEdgesIdxs[0]][0]);
	for (i32 i = 1; i < faceEdgesIdxs.size(); i++) {
		const auto& edge = edges[faceEdgesIdxs[i]];
		if (vertices.back() == edge[0]) {
			vertices.push_back(edge[1]);
		} else {
			vertices.push_back(edge[0]);
		}
	}
	return vertices;
}

Polytope::CellsN& Polytope::cellsOfDimension(i32 n) {
	return cells[n - 1];
}

const Polytope::CellsN& Polytope::cellsOfDimension(i32 n) const {
	return cells[n - 1];
}
