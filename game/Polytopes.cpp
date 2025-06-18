#include "Polytopes.hpp"
#include "Combinatorics.hpp"
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <array>
#include <game/ConvexHull.hpp>
#include <engine/Math/Quat.hpp>
#include <game/600cell.hpp>
#include <StaticList.hpp>
#include <View.hpp>

void addPyramid(
	Polytope& polytope, 
	i32 pyramidVertexIndex, 
	// This is used for example for making a bipyramid. The procedure shouldn't connect the vertices added in the first call to the pyramid vertex so this only iterates for the ones with indices lower than the bound.
	const std::vector<i32>& upperIndexBoundsForBase) {
	const auto baseDimension = upperIndexBoundsForBase.size();

	// The 1-cell coming out of vertex with index i will have index newCellsOffset + i.
	i32 newCellsOffset = i32(polytope.cellsOfDimension(1).size());
	for (i32 vertexI = 0; vertexI < upperIndexBoundsForBase[0]; vertexI++) {
		auto& cells = polytope.cellsOfDimension(1);
		cells.push_back(Polytope::CellN{ vertexI, pyramidVertexIndex });
	}
	for (i32 dimension = 2; dimension < baseDimension + 1; dimension++) {
		auto& dimensionCells = polytope.cellsOfDimension(dimension);
		const auto cellOffset = i32(polytope.cellsOfDimension(dimension).size());

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
	const auto apex0 = i32(result.vertices.size()) - 2;
	const auto apex1 = i32(result.vertices.size()) - 1;
	
	std::vector<i32> upperIndexBoundsForBase;
	upperIndexBoundsForBase.push_back(i32(result.vertices.size()) - 2);
	for (const auto& cells : result.cells) {
		upperIndexBoundsForBase.push_back(i32(cells.size()));
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
			const auto offset = i32(baseSubCells.size()) * 2;
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

f32 dot(Quat a, Quat b) {
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

Quat slerp(Quat q1, Quat q2, f32 t) {
	const auto angle = acos(dot(q1, q2));
	const auto denom = sin(angle);
	//check if denom is zero
	if (denom < 0.001f) {
		return Quat::identity;
	}
	return (q1 * sin((1 - t) * angle) + q2 * sin(t * angle)) / denom;
}

void subdivideCube(
	Polytope& p, i32 divisionCount, 
	Quat v000, Quat v001, Quat v010, Quat v011, // bottom
	Quat v100, Quat v101, Quat v110, Quat v111 // top
) {
	const auto verticesOffset = i32(p.vertices.size());
	i32 vertexCountPerEdge = divisionCount + 2;
	for (i32 i = 0; i < vertexCountPerEdge; i++) {
		const auto it = f32(i) / f32(vertexCountPerEdge - 1);

		const auto a0 = slerp(v000, v001, it);
		const auto a1 = slerp(v010, v011, it);

		const auto a2 = slerp(v100, v101, it);
		const auto a3 = slerp(v110, v111, it);

		for (i32 j = 0; j < vertexCountPerEdge; j++) {
			const auto jt = f32(j) / f32(vertexCountPerEdge - 1);

			const auto b0 = slerp(a0, a1, jt);
			const auto b1 = slerp(a2, a3, jt);

			for (i32 k = 0; k < vertexCountPerEdge; k++) {
				const auto kt = f32(k) / f32(vertexCountPerEdge - 1);

				const auto c = slerp(b0, b1, kt);
				p.vertices.push_back({ c.x, c.y, c.z, c.w });
			}
		}
	}

	auto index = [&](i32 x, i32 y, i32 z) -> i32 {
		return verticesOffset + x * vertexCountPerEdge * vertexCountPerEdge + y * vertexCountPerEdge + z;
	};

	auto addCube = [&](
		i32 i000, i32 i001, i32 i010, i32 i011, // bottom
		i32 i100, i32 i101, i32 i110, i32 i111 // top
	) {
		auto& edges = p.cellsOfDimension(1);
		const auto edgesOffset = i32(edges.size());
		// bottom
		edges.push_back({ i000, i001 }); // 0
		edges.push_back({ i010, i011 }); // 1
		edges.push_back({ i000, i010 }); // 2
		edges.push_back({ i001, i011 }); // 3

		// top
		edges.push_back({ i100, i101 }); // 4
		edges.push_back({ i110, i111 }); // 5
		edges.push_back({ i100, i110 }); // 6
		edges.push_back({ i101, i111 }); // 7

		// sides
		edges.push_back({ i000, i100 }); // 8
		edges.push_back({ i001, i101 }); // 9
		edges.push_back({ i010, i110 }); // 10
		edges.push_back({ i011, i111 }); // 11

		auto& faces = p.cellsOfDimension(2);
		const auto facesOffset = i32(faces.size());
		auto eo = [&](i32 i) -> i32 { return i + edgesOffset; };
		// top, bottom
		faces.push_back({ eo(0), eo(1), eo(2), eo(3) });
		faces.push_back({ eo(4), eo(5), eo(6), eo(7) });
		/*
		Algorithm: 
		Pick an edge on the bottom, pick the corespoding edge on the top. That is the edge with index + 4. Then select to hilight all the occurences of the first vertex and add that highlighted add in the sides section do the same for the second vertex.
		*/
		faces.push_back({ eo(0), eo(4), eo(8), eo(9) });
		faces.push_back({ eo(1), eo(5), eo(10), eo(11) });
		faces.push_back({ eo(2), eo(6), eo(8), eo(10) });
		faces.push_back({ eo(3), eo(7), eo(9), eo(11) });

		auto& cells = p.cellsOfDimension(3);
		const auto cellOffset = cells.size();
		auto fo = [&](i32 i) -> i32 { return i + facesOffset; };
		cells.push_back({ fo(0), fo(1), fo(2), fo(3), fo(4), fo(5) });
		//cells.push_back({ 0 });
	};

	for (i32 i = 0; i < vertexCountPerEdge - 1; i++) {
		for (i32 j = 0; j < vertexCountPerEdge - 1; j++) {
			for (i32 k = 0; k < vertexCountPerEdge - 1; k++) {
				addCube(
					index(i, j, k),
					index(i, j, k + 1),
					index(i, j + 1, k),
					index(i, j + 1, k + 1),
					index(i + 1, j, k),
					index(i + 1, j, k + 1),
					index(i + 1, j + 1, k),
					index(i + 1, j + 1, k + 1)
				);
			}
		}
	}
}

#include <iostream>

Polytope removedDuplicates(Polytope&& p) {
	Polytope result;
	std::vector<i32> oldToNew;

	//oldToNew.resize(p.vertices.size());
	oldToNew.clear();
	for (auto& vertex : p.vertices) {
		bool alreadyAdded = false;
		for (i32 alreadyAddedVertexI = 0; alreadyAddedVertexI < result.vertices.size(); alreadyAddedVertexI++) {
			const auto& alreadyAddedVertex = result.vertices[alreadyAddedVertexI];

			auto dist = [](const std::vector<f32>& a, const std::vector<f32>& b) {
				f32 result = 0.0f;
				for (i32 i = 0; i < a.size(); i++) {
					result += pow(a[i] - b[i], 2.0f);
				}
				return result;
			};
			if (dist(vertex, alreadyAddedVertex) < 0.01f) {
				oldToNew.push_back(alreadyAddedVertexI);
				alreadyAdded = true;
				break;
			}
		}
		if (!alreadyAdded) {
			oldToNew.push_back(i32(result.vertices.size()));
			result.vertices.push_back(std::move(vertex));
		}
	}

	struct Hash {
		const std::vector<Polytope::CellN>& cells;

		usize operator ()(i32 keyIndex) const {
			auto& k = cells[keyIndex];
			usize hash = std::hash<i32>()(k[0]);
			for (i32 i = 1; i < k.size() - 1; i++) {
				hash = hashCombine(hash, std::hash<i32>()(k[i]));
			}
			return hash;
		}
	};

	struct Eq {
		const std::vector<Polytope::CellN>& cells;

		bool operator()(i32 a, i32 b) const {
			return cells[a] == cells[b];
		}
	};


	for (auto& cells : p.cells) {
		result.cells.push_back(Polytope::CellsN());
		auto& resultCells = result.cells.back();

		Hash hash{ cells };
		Eq eq{ cells };

		std::unordered_map<i32, i32, Hash, Eq> oldCellToNew(10, hash, eq);

		for (auto& cell : cells) {
			for (auto& subCellIndex : cell) {
				subCellIndex = oldToNew[subCellIndex];
			}
			// Sort the indices in the cell to make equality testing easier.
			std::ranges::sort(cell);
		}
		oldToNew.clear();
		for (i32 cellI = 0; cellI < cells.size(); cellI++) {
			const auto& cell = cells[cellI];
			const auto newIndex = i32(resultCells.size()); 
			//const auto r = oldCellToNew.find(cellI);
			const auto r = oldCellToNew.try_emplace(cellI, newIndex);
			const auto insertedNew = r.second;
			if (insertedNew) {
				oldToNew.push_back(newIndex);
				resultCells.push_back(cell);
			} else {
				oldToNew.push_back(r.first->second);
			}
		}
		//for (auto& cell : cells) {
		//	oldVertexToNew.find()

		//	/*bool alreadyAdded = false;
		//	for (i32 alreadyAddedCellI = 0; alreadyAddedCellI < resultCells.size(); alreadyAddedCellI++) {
		//		const auto& alreadyAddedCell = resultCells[alreadyAddedCellI];

		//		auto equals = [](const std::vector<i32>& a, const std::vector<i32>& b) {
		//			ASSERT(a.size() == b.size());
		//			return a == b;
		//		};
		//		if (equals(cell, alreadyAddedCell)) {
		//			oldToNew.push_back(alreadyAddedCellI);
		//			alreadyAdded = true;
		//			break;
		//		}
		//	}*/


		//	if (!alreadyAdded) {
		//		oldToNew.push_back(i32(resultCells.size()));
		//		resultCells.push_back(std::move(cell));
		//	}
		//}
	}
	return result;
}

Polytope subdiviedHypercube4(i32 divisionCount) {
	const auto h = hypercube(4);
	/*
	(4 choose 3) places for the changing 1 and -1
	and the other place for a constant 1
	*/
	Polytope result;
	result.cells.resize(3);
	const auto p = 0.5f;
	const auto m = -0.5f;
	f32 ds[]{ p, m };
	for (const auto d : ds) {
		for (i32 i = 0; i < 4; i++) {
			auto c = [&](f32 x, f32 y, f32 z) -> Quat {
				f32 a[]{ x, y, z };
				f32 b[4]{};
				b[i] = 1.0f;
				i32 j = 0;
				b[i] = d;
				for (i32 k = 0; k < 4; k++) {
					if (k != i) {
						b[k] = a[j];
						j++;
					}
				}
				return Quat(b[0], b[1], b[2], b[3]);
			};

			subdivideCube(
				result, divisionCount,
				c(m, m, m), c(m, m, p), c(m, p, m), c(m, p, p),
				c(p, m, m), c(p, m, p), c(p, p, m), c(p, p, p)
			);
			//goto zend;
		}
	}
	return removedDuplicates(std::move(result));
}

Polytope::CellN faceEdgesSorted(const Polytope& p, i32 faceIndex) {
	auto& faceEdgesIndices = p.cellsOfDimension(2)[faceIndex];
	auto& edges = p.cellsOfDimension(1);
	std::vector<bool> visitedEdges;
	visitedEdges.resize(faceEdgesIndices.size(), false);
	Polytope::CellN sortedEdgesIndices;

 	i32 currentEdgeIndex = faceEdgesIndices[0];
	i32 currentVertex = edges[currentEdgeIndex][0];
	visitedEdges[0] = true;
	sortedEdgesIndices.push_back(currentEdgeIndex);

	while (sortedEdgesIndices.size() < faceEdgesIndices.size()) {

		auto& currentEdge = edges[currentEdgeIndex];

		auto nextVertex = currentEdge[0];
		if (currentVertex == nextVertex) {
			nextVertex = currentEdge[1];
		}

		bool foundNextVertex = false;
		for (i32 i = 0; i < faceEdgesIndices.size(); i++) {
			auto& edgeIndex = faceEdgesIndices[i];
			if (edgeIndex == currentEdgeIndex) {
				continue;
			}
			auto& edge = edges[edgeIndex];
			if (edge[0] == nextVertex || edge[1] == nextVertex) {
				currentVertex = nextVertex;
				if (visitedEdges[i]) {
					CHECK_NOT_REACHED();
					return faceEdgesIndices;
				}
				currentEdgeIndex = edgeIndex;
				sortedEdgesIndices.push_back(edgeIndex);
				visitedEdges[i] = true;
				foundNextVertex = true;
				break;
			}
		}
		if (!foundNextVertex) {
			CHECK_NOT_REACHED();
			return faceEdgesIndices;
		}
	}
	return sortedEdgesIndices;
}

struct Vec4SignSwitchIterator {
	Vec4 v;
	Vec4SignSwitchIterator(f32 x, f32 y, f32 z, f32 w, bool switchSignX, bool switchSignY, bool switchSignZ, bool switchSignW);
	Vec4SignSwitchIterator(Vec4 v, bool switchSignX, bool switchSignY, bool switchSignZ, bool switchSignW);
	Vec4SignSwitchIterator(Vec4 v, StaticList<i32, 4> indicesToSwitch, i32 i);
	StaticList<i32, 4> indicesToSwitch;
	i32 i = 0;

	Vec4SignSwitchIterator begin();
	Vec4SignSwitchIterator end();
	Vec4 operator*() const;
	bool operator==(const Vec4SignSwitchIterator& other) const;
	bool operator!=(const Vec4SignSwitchIterator& other) const;

	Vec4SignSwitchIterator& operator++();
};

struct HashVec4 {
	usize operator()(Vec4 v) const {
		return hashCombine(v.x, hashCombine(v.y, hashCombine(v.z, v.w)));
	}
};
using VertexSet = std::unordered_set<Vec4, HashVec4>;
std::vector<Vec4> vertexSetToVertexList(const VertexSet& v) {
	std::vector<Vec4> r;
	for (const auto& vertex : v) {
		r.push_back(vertex);
	}
	return r;
}

bool isPermuationEven(const std::array<i32, 4>& p) {
	i32 inversionCount = 0;
	for (i32 i = 0; i < p.size(); i++) {
		for (i32 j = i + 1; j < p.size(); j++) {
			if (p[i] > p[j]) inversionCount++;
		}
	}
	return inversionCount % 2;
}

void addPermuations(VertexSet& vertices, f32 x, f32 y, f32 z, f32 w, bool switchSignX, bool switchSignY, bool switchSignZ, bool switchSignW, bool addOnlyEvenPermuations = false) {
	for (const auto& v : Vec4SignSwitchIterator(x, y, z, w, switchSignX, switchSignY, switchSignZ, switchSignW)) {
		std::array<i32, 4> permuation{ 0, 1, 2, 3 };
		//i32 i = 0;
		do {
			if (!addOnlyEvenPermuations || isPermuationEven(permuation)) {
				//CHECK(isPermuationEven(permuation));
				Vec4 permutedV(v[permuation[0]], v[permuation[1]], v[permuation[2]], v[permuation[3]]);
				vertices.insert(permutedV);
			}
			//i++;
		} while (std::next_permutation(permuation.begin(), permuation.end()));
	}
}

void addPermuations(VertexSet& vertices, f32 x, f32 y, f32 z, f32 w, bool addOnlyEvenPermuations = false) {
	bool switchSignX = x != 0.0f;
	bool switchSignY = y != 0.0f;
	bool switchSignZ = z != 0.0f;
	bool switchSignW = w != 0.0f;
	for (const auto& v : Vec4SignSwitchIterator(x, y, z, w, switchSignX, switchSignY, switchSignZ, switchSignW)) {
		std::array<i32, 4> permuation{ 0, 1, 2, 3 };
		//i32 i = 0;
		do {
			if (!addOnlyEvenPermuations || isPermuationEven(permuation)) {
				//CHECK(isPermuationEven(permuation));
				Vec4 permutedV(v[permuation[0]], v[permuation[1]], v[permuation[2]], v[permuation[3]]);
				vertices.insert(permutedV);
			}
			//i++;
		} while (std::next_permutation(permuation.begin(), permuation.end()));
	}
}


void add24CellVertices(VertexSet& vertices) {
	addPermuations(vertices, 0.0f, 0.0f, 2.0f, 2.0f, 0, 0, 1, 1);
}

Polytope make600cell() {
	{
		const auto p = (1.0f + sqrt(5.0f)) / 2.0f;
		VertexSet v;
		addPermuations(v, 0, 0, 0, 1);
		addPermuations(v, 0.5f, 0.5f, 0.5f, 0.5f);
		addPermuations(v, p / 2.0f, 0.5f, 1.0f / (2.0f * p), 0.0f, true);
		/*const auto p = (1.0f + sqrt(5.0f)) / 2.0f;
		const auto p2 = p * p;
		const auto pm1 = 1.0f / p;
		const auto pm2 = 1.0f / p2;
		const auto s5 = sqrt(5.0f);
		add24CellVertices(v);
		addPermuations(v, p, p, p, pm2, 1, 1, 1, 1);
		addPermuations(v, 1.0f, 1.0f, 1.0f, s5, 1, 1, 1, 1);
		addPermuations(v, pm1, pm1, pm1, p2, 1, 1, 1, 1);
		addPermuations(v, 0.0f, pm1, p, s5, 0, 1, 1, 1, true);
		addPermuations(v, 0.0f, pm2, 1.0f, p2, 0, 1, 1, 1, true);
		addPermuations(v, pm1, 1.0f, p, 2.0f, 1, 1, 1, 1, true);*/
		const auto vertices = vertexSetToVertexList(v);
		return convexHull(vertices);
	}

	Polytope r;
	//for (i32 i = 0; i < std::size(vertices600cell); i++) {
	//	const auto& p = vertices600cell[i];
	//	r.vertices.push_back(Polytope::PointN{ p.x, p.y, p.z, p.w });
	//}
	//auto addCells = [](Polytope::CellsN& out, View<const i32> in, i32 subCellsPerCell) {
	//	for (i32 i = 0; i < in.size(); i += subCellsPerCell) {
	//		Polytope::CellN cell;
	//		for (i32 j = 0; j < subCellsPerCell; j++) {
	//			cell.push_back(in[i + j]);
	//		}
	//		out.push_back(std::move(cell));
	//	}
	//};
	//r.cells.push_back(Polytope::CellsN{});
	//auto& edges = r.cells.back();
	//addCells(edges, constView(edges600cell), 2);

	//r.cells.push_back(Polytope::CellsN{});
	//auto& faces = r.cells.back();
	//addCells(faces, constView(faces600cell), 3);

	//r.cells.push_back(Polytope::CellsN{});
	//auto& cells = r.cells.back();
	//addCells(cells, constView(cells600cell), 4);


	for (i32 i = 0; i < std::size(vertices600cell); i++) {
		const auto& p = vertices600cell[i];
		r.vertices.push_back(Polytope::PointN{ p.x, p.y, p.z, p.w });
	}
	auto addCells = [](Polytope::CellsN& out, i32 cellCount, View<const i32> subCellToCells, i32 cellsPerSubCell) {
		out.resize(cellCount);

		for (i32 subCellI = 0; subCellI < subCellToCells.size() / cellsPerSubCell; subCellI++) {
			for (i32 i = 0; i < cellsPerSubCell; i++) {
				auto cellI = subCellToCells[subCellI * cellsPerSubCell + i];
				out[cellI].push_back(subCellI);
			}
		}
		/*for (i32 i = 0; i < in.size(); i += subCellsPerCell) {
			Polytope::CellN cell;
			for (i32 j = 0; j < subCellsPerCell; j++) {
				cell.push_back(in[i + j]);
			}
			out.push_back(std::move(cell));
		}*/
		};
	r.cells.push_back(Polytope::CellsN{});
	auto& edges = r.cells.back();
	for (i32 i = 0; i < std::size(edges600cell); i += 2) {
		Polytope::CellN cell;
		for (i32 j = 0; j < 2; j++) {
			cell.push_back(edges600cell[i + j]);
		}
		edges.push_back(std::move(cell));
	}

	r.cells.push_back(Polytope::CellsN{});
	auto& faces = r.cells.back();
	addCells(faces, i32(std::size(faces600cell)) / 3, constView(edgeToFaces600cell), 5);

	r.cells.push_back(Polytope::CellsN{});
	auto& cells = r.cells.back();
	addCells(cells, i32(std::size(cells600cell)) / 4, constView(faceToCells600cell), 2);

	return r;
}

Polytope make120cell() {
	//https://en.wikipedia.org/wiki/120-cell#%E2%88%9A8_radius_coordinates
	VertexSet v;
	const auto p = (1.0f + sqrt(5.0f)) / 2.0f;
	const auto p2 = p * p;
	const auto pm1 = 1.0f / p;
	const auto pm2 = 1.0f / p2;
	const auto s5 = sqrt(5.0f);
	add24CellVertices(v);
	addPermuations(v, p, p, p, pm2, 1, 1, 1, 1);
	addPermuations(v, 1.0f, 1.0f, 1.0f, s5, 1, 1, 1, 1);
	addPermuations(v, pm1, pm1, pm1, p2, 1, 1, 1, 1);
	addPermuations(v, 0.0f, pm1, p, s5, 0, 1, 1, 1, true);
	addPermuations(v, 0.0f, pm2, 1.0f, p2, 0, 1, 1, 1, true);
	addPermuations(v, pm1, 1.0f, p, 2.0f, 1, 1, 1, 1, true);
	const auto vertices = vertexSetToVertexList(v);
	return convexHull(vertices);
}

Polytope make24cell() {
	VertexSet v;
	add24CellVertices(v);
	const auto vertices = vertexSetToVertexList(v);
	return convexHull(vertices);
}

Polytope make5cell() {
	// https://en.wikipedia.org/wiki/5-cell#Coordinates
	std::vector<Vec4> vertices;
	const auto p = (1.0f + sqrt(5.0f)) / 2.0f;
	vertices.push_back(Vec4(2.0f, 0.0f, 0.0f, 0.0f));
	vertices.push_back(Vec4(0.0f, 2.0f, 0.0f, 0.0f));
	vertices.push_back(Vec4(0.0f, 0.0f, 2.0f, 0.0f));
	vertices.push_back(Vec4(0.0f, 0.0f, 0.0f, 2.0f));
	vertices.push_back(Vec4(p, p, p, p));
	for (auto& vertex : vertices) {
		vertex -= Vec4(1.0f, 1.0f, 1.0f, 1.0f) / (2.0f - 1.0f / p);
	}
	return convexHull(vertices);
}

Polytope makeRectified5cell() {
	// https://en.wikipedia.org/wiki/Rectified_5-cell#Coordinates
	std::vector<Vec4> v;

	const auto s2o5 = sqrt(2.0f / 5.0f);
	const auto s1o6 = sqrt(1.0f / 6.0f);
	const auto s1o3 = sqrt(1.0f / 3.0f);

	v.push_back(Vec4(s2o5, 2.0f * s1o6, 2.0f * s1o3, 0.0f));

	v.push_back(Vec4(s2o5, 2.0f * s1o6, -1.0f * s1o3, 1.0f));
	v.push_back(Vec4(s2o5, 2.0f * s1o6, -1.0f * s1o3, -1.0f));

	v.push_back(Vec4(s2o5, -2.0f * s1o6, 1.0f * s1o3, 1.0f));
	v.push_back(Vec4(s2o5, -2.0f * s1o6, 1.0f * s1o3, -1.0f));

	v.push_back(Vec4(s2o5, -2.0f * s1o6, -2.0f * s1o3, 0.0f));

	v.push_back(Vec4(-3.0f / sqrt(10.0f), s1o6, s1o3, 1.0f));
	v.push_back(Vec4(-3.0f / sqrt(10.0f), s1o6, s1o3, -1.0f));

	v.push_back(Vec4(-3.0f / sqrt(10.0f), s1o6, -2.0f * s1o3, 0.0f));

	v.push_back(Vec4(-3.0f / sqrt(10.0f), -sqrt(3.0f / 2.0f), 0.0f, 0.0f));
	return convexHull(v);
}

Polytope makeRectified600cell() {
	// https://www.qfbox.info/4d/rect600cell
	VertexSet v;
	const auto p = (1.0f + sqrt(5.0f)) / 2.0f;
	const auto p2 = p * p;
	const auto p3 = p2 * p;
	addPermuations(v, 0, 0, 2 * p, 2 * p2);
	addPermuations(v, 1, 1, p3, p3);
	addPermuations(v, 0, 1, p, 1 + 3 * p, true);
	addPermuations(v, 0, p2, p3, 2 + p, true);
	addPermuations(v, 1, p, 2 * p2, p2, true);
	addPermuations(v, p, p2, 2 * p, p3, true);
	return convexHull(vertexSetToVertexList(v));
}

Polytope makeSnub24cell() {
	// https://www.qfbox.info/4d/snub24cell
	VertexSet v;
	const auto p = (1.0f + sqrt(5.0f)) / 2.0f;
	const auto p2 = p * p;
	addPermuations(v, 0.0f, 1.0f, p, p2, true);
	return convexHull(vertexSetToVertexList(v));
}

std::vector<i32> verticesOfFaceWithSortedEdges(const Polytope& p, i32 faceIndex) {
	return verticesOfFaceWithSortedEdges(p, p.cellsOfDimension(2)[faceIndex]);
}

std::vector<i32> verticesOfFaceWithSortedEdges(const Polytope& p, const Polytope::CellN& face) {
	const auto& faceEdgesIdxs = face;
	const auto& edges = p.cellsOfDimension(1);

	i32 startIndex = 0;
	std::vector<i32> vertices;

	vertices.push_back(edges[faceEdgesIdxs[0]][0]);
	vertices.push_back(edges[faceEdgesIdxs[0]][1]);
	for (i32 i = 1; i < faceEdgesIdxs.size() - 1; i++) {
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

Vec4SignSwitchIterator::Vec4SignSwitchIterator(f32 x, f32 y, f32 z, f32 w, bool switchSignX, bool switchSignY, bool switchSignZ, bool switchSignW) 
	: Vec4SignSwitchIterator(Vec4(x, y, z, w), switchSignX, switchSignY, switchSignZ, switchSignW) {}

Vec4SignSwitchIterator::Vec4SignSwitchIterator(Vec4 v, bool switchSignX, bool switchSignY, bool switchSignZ, bool switchSignW)
	: v(v) {
	if (switchSignX) indicesToSwitch.add(0);
	if (switchSignY) indicesToSwitch.add(1);
	if (switchSignZ) indicesToSwitch.add(2);
	if (switchSignW) indicesToSwitch.add(3);
}

Vec4SignSwitchIterator::Vec4SignSwitchIterator(Vec4 v, StaticList<i32, 4> indicesToSwitch, i32 i)
	: v(v)
	, indicesToSwitch(indicesToSwitch)
	, i(i) {}

Vec4SignSwitchIterator Vec4SignSwitchIterator::begin() {
	auto copy = *this;
	copy.i = 0;
	return copy;
}

Vec4SignSwitchIterator Vec4SignSwitchIterator::end() {
	auto copy = *this;
	copy.i = integerToNonNegativePower(2, i32(copy.indicesToSwitch.size()));
	return copy;
}

Vec4 Vec4SignSwitchIterator::operator*() const {
	Vec4 result = v;
	for (i32 j = 0; j < indicesToSwitch.size(); j++) {
		f32 sign = (i >> j & 1) ? -1.0f : 1.0f;
		result[indicesToSwitch[j]] *= sign;
	}
	return result;
}

bool Vec4SignSwitchIterator::operator==(const Vec4SignSwitchIterator& other) const {
	CHECK(v == other.v);
	return i == other.i;
}

bool Vec4SignSwitchIterator::operator!=(const Vec4SignSwitchIterator& other) const {
	CHECK(v == other.v);
	return i != other.i;
}

Vec4SignSwitchIterator& Vec4SignSwitchIterator::operator++() {
	i++;
	return *this;
}
