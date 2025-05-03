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
	return integerToNonNegativePower(2, k + 1) * nChooseK(n, k + 1);

	//for (i32 dimensionOfPolytope = 2; dimensionOfPolytope < 10; dimensionOfPolytope++) {
	//	const auto polytope = crossPolytope(dimensionOfPolytope);
	//	for (i32 i = 0; i < polytope.cells.size(); i++) {
	//		const auto dimensionOfSimplex = i + 1;
	//		const auto expectedSize = crossPolytopeSimplexCount(dimensionOfPolytope, dimensionOfSimplex);
	//		if (polytope.cells[i].size() != expectedSize) {
	//			ASSERT_NOT_REACHED();
	//		}
	//	}
	//}
}

Polytope hypercube(i32 polytopeDimension) {
	ASSERT(polytopeDimension >= 2);
	if (polytopeDimension == 2) {
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

	auto base = hypercube(polytopeDimension - 1);
	{
		// For example if the base is a square then it is described by it's edges. This also adds the square as a face.
		base.cells.push_back(Polytope::CellsN());
		base.cells.back().push_back(Polytope::CellN());
		auto& baseCell = base.cells.back().back();
		auto& baseSubCells = base.cells[base.cells.size() - 2];
		for (i32 i = 0; i < baseSubCells.size(); i++) {
			baseCell.push_back(i);
		}
	}

	// The vertices are ordered as top0, bottom0, top1, bottom1, ...
	Polytope result;
	for (auto& vertex : base.vertices) {
		result.vertices.push_back(vertex);
		result.vertices.back().push_back(1.0f);
		result.vertices.push_back(vertex);
		result.vertices.back().push_back(-1.0f);
	}
	// the cells are ordered as
	// top0, bottom0, top1, bottom1, ..., 
	// after that are the cells correspoding to cells of lower dimension.
	// so for for example the top and bottom vertex connected.
	for (i32 dimension = 1; dimension < polytopeDimension; dimension++) {
		result.cells.push_back(Polytope::CellsN());
		auto& resultCells = result.cells.back();
		//auto& resultCells = result.cellsOfDimension(dimension);
		const auto& baseCells = base.cellsOfDimension(dimension);
		for (const auto& baseCell : baseCells) {
			for (i32 i = 0; i < 2; i++) {
				Polytope::CellN resultCell;
				for (auto& subCell : baseCell) {
					resultCell.push_back(subCell * 2 + i);
				}
				resultCells.push_back(std::move(resultCell));
			}
			//for (auto& subCell : baseCell) {
			//	Polytope::CellN resultCell;
			//	for (auto& )
			//	//resultCell.push_back(subCell * 2 + i);
			//}
			//resultCells.push_back()
		}
		if (dimension - 1 == 0) {
			for (i32 vertexI = 0; vertexI < base.vertices.size(); vertexI++) {
				Polytope::CellN edge;
				edge.push_back(vertexI * 2 + 0);
				edge.push_back(vertexI * 2 + 1);
				resultCells.push_back(edge);
			}
		} else {
			const auto& baseSubCells = base.cellsOfDimension(dimension - 1);
			// offset to cell of 2 dimensions lower.
			// so if we are making a faces that is if dimension = 2
			// then this will itrate over the edges and makes faces from the top edge bottom edge and edges correspoding to each vertex of that edge.
			const auto offset = baseSubCells.size() * 2;
			for (i32 baseSubCellI = 0; baseSubCellI < baseSubCells.size(); baseSubCellI++) {
				Polytope::CellN resultCell;
				resultCell.push_back(baseSubCellI * 2 + 0);
				resultCell.push_back(baseSubCellI * 2 + 1);
				auto& baseSubCell = baseSubCells[baseSubCellI];
				for (const auto& subSubCells : baseSubCell) {
					resultCell.push_back(offset + subSubCells);
				}
				resultCells.push_back(std::move(resultCell));
			}
		}
	}
	return result;
}
const auto polytope = hypercube(4);

i32 hypercubeCellCount(i32 dimensionOfHypercube, i32 dimensionOfCells) {
	const auto& n = dimensionOfHypercube;
	const auto& k = dimensionOfCells;
	// S(n, k) = 2 ^ (n - k) (n choose k)
	return integerToNonNegativePower(2, n - k) * nChooseK(n, k);
	//for (i32 dimensionOfPolytope = 2; dimensionOfPolytope < 10; dimensionOfPolytope++) {
	//	const auto polytope = hypercube(dimensionOfPolytope);
	//	for (i32 i = 0; i < dimensionOfPolytope - 1; i++) {
	//		const auto dimensionOfCell = i + 1;
	//		const auto expectedSize = hypercubeCellCount(dimensionOfPolytope, dimensionOfCell);
	//		if (polytope.cells[i].size() != expectedSize) {
	//			ASSERT_NOT_REACHED();
	//		}
	//	}
	//}
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
