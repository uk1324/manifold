#include "ConvexHull.hpp"
#include <dependencies/qhull/src/libqhullcpp/Qhull.h>
#include <dependencies/qhull/src/libqhullcpp/QhullError.h>
#include <libqhullcpp/QhullFacetList.h>
#include <libqhullcpp/QhullRidge.h>
#include <unordered_map>
#include <unordered_set>

// https://stackoverflow.com/questions/19530731/qhull-library-c-interface
Polytope convexHull(const std::vector<Vec4>& points) {
	std::vector<double> pointsData;
	for (const auto& point : points) {
		pointsData.push_back(point.x);
		pointsData.push_back(point.y);
		pointsData.push_back(point.z);
		pointsData.push_back(point.w);
	}

	// http://www.qhull.org/html/rbox.htm
	std::string comment = "";

	// http://www.qhull.org/html/qhull.htm
	// Precision options:
	// Cn - radius of centrum (roundoff added). Merge facets if non-convex
	std::string command = "C0.001";

	const auto dimension = 4;
	const auto pointsCount = i32(points.size());
	try {
		orgQhull::Qhull hull(
			comment.c_str(), 
			dimension,
			pointsCount, 
			pointsData.data(),
			command.c_str()
		);
		std::vector<std::vector<i32>> facesVertices;
		std::vector<std::vector<i32>> facesCellsTheyBelongTo;

		// The ids are not ordered
		//for (const auto& vertex : hull.vertexList()) {
		//	std::cout << vertex.id() << '\n';
		//}
		std::unordered_map<i32, i32> vertexIdToVertexIndex;
		{
			i32 i = 0;
			for (const auto& vertex : hull.vertexList()) {
				vertexIdToVertexIndex[vertex.id()] = i;
				i++;
			}
		}

		const auto cellCount = hull.facetCount();
		const auto& cells = hull.facetList();
		i32 cellAI = 0;
		for (auto cellA = cells.begin(); cellA != cells.end(); ++cellA) {
			i32 cellBI = cellAI + 1;
			auto cellB = cellA;
			++cellB;
			for (; cellB != cells.end(); ++cellB) {
				std::vector<i32> sharedVertices;
				for (const auto& vertexA : cellA->vertices()) {
					for (const auto& vertexB : cellB->vertices()) {
						if (vertexA == vertexB) {
							sharedVertices.push_back(vertexIdToVertexIndex[vertexA.id()]);
						}
					}
				}
				if (sharedVertices.size() >= 3) {
					facesVertices.push_back(std::move(sharedVertices));
					facesCellsTheyBelongTo.push_back({});
					facesCellsTheyBelongTo.back().push_back(cellAI);
					facesCellsTheyBelongTo.back().push_back(cellBI);
				}
				cellBI++;
			}
			cellAI++;
		}

		// Quite likely there will be duplicates.
		std::vector<std::vector<i32>> edgesVertices;
		std::vector<std::vector<i32>> edgesFacesTheyBelongTo;
		for (i32 a = 0; a < facesVertices.size(); a++) {
			const auto& faceAVertices = facesVertices[a];
			for (i32 b = a + 1; b < facesVertices.size(); b++) {
				const auto& faceBVertices = facesVertices[b];
				std::vector<i32> sharedVertices;

				for (const auto& vertexA : faceAVertices) {
					for (const auto& vertexB : faceBVertices) {
						if (vertexA == vertexB) {
							sharedVertices.push_back(vertexA);
						}
					}
				}
				if (sharedVertices.size() == 2) {
					// faces can share no vertices a single vertex or 2 vertices.
					edgesVertices.push_back(std::move(sharedVertices));
					edgesFacesTheyBelongTo.push_back({ a, b });
				}
			}
		}
		// Sort to make checking equality faster.
		for (auto& edge : edgesVertices) {
			std::ranges::sort(edge);
		}
		struct Hash {
			usize operator()(const std::vector<i32>& v) const {
				if (v.size() == 0) {
					CHECK_NOT_REACHED();
					return 0;
				}
				usize hash = v[0];
				for (i32 i = 1; i < v.size(); i++) {
					hash = hashCombine(hash, i);
				}
				return hash;
			}
		};
		std::unordered_map<std::vector<i32>, std::unordered_set<i32>, Hash> uniqueEdgesToFacesTheyBelongTo;
		for (i32 i = 0; i < edgesVertices.size(); i++) {
			auto& edge = edgesVertices[i];
			const auto entry = uniqueEdgesToFacesTheyBelongTo.find(edge);
			const auto& faces = edgesFacesTheyBelongTo[i];
			if (entry == uniqueEdgesToFacesTheyBelongTo.end()) {
				std::unordered_set<i32> set;
				set.insert_range(faces);
				uniqueEdgesToFacesTheyBelongTo.insert({ std::move(edge), std::move(set) });
			} else {
				for (const auto& face : faces) {
					entry->second.insert(face);
				}
			}
			/*auto& edge = edgesVertices[i];
			uniqueEdgesToFacesTheyBelongTo.try_emplace(std::move(edge), )*/
		}

		Polytope result;

		for (const auto& vertex : hull.vertexList()) {
			Polytope::PointN v;
			for (i32 i = 0; i < 4; i++) {
				v.push_back(f32(vertex.point()[i]));
			}
			result.vertices.push_back(std::move(v));
		}

		result.cells.push_back(Polytope::CellsN{});
		//Polytope::CellsN& edgesVertices = result.cells.back();
		for (const auto& [edgeVertices, _] : uniqueEdgesToFacesTheyBelongTo) {
			/*std::vector<i32> v;
			for (const auto& e : edgeVertices) {
				v.push_back(e - 1);
			}
			result.cells.back().push_back(std::move(v));*/
			// Copy, because you can't move out they key of an unodered_map.
			result.cells.back().push_back(Polytope::CellN(edgeVertices));
		}

		result.cells.push_back(Polytope::CellsN{});
		auto& facesEdges = result.cells.back();
		facesEdges.resize(facesVertices.size());
		i32 edgeI = 0;
		for (const auto& [_, edgeFaces] : uniqueEdgesToFacesTheyBelongTo) {
			for (const auto& faceI : edgeFaces) {
				facesEdges[faceI].push_back(edgeI);
			}
			edgeI++;
		}

		result.cells.push_back(Polytope::CellsN{});
		auto& cellsFaces = result.cells.back();
		cellsFaces.resize(cells.size());
		for (i32 faceI = 0; faceI < facesCellsTheyBelongTo.size(); faceI++) {
			for (const auto& cellI : facesCellsTheyBelongTo[faceI]) {
				cellsFaces[cellI].push_back(faceI);
			}
		}
		return result;
	} catch (orgQhull::QhullError& e) {
		CHECK_NOT_REACHED();
		return Polytope();
	}
}
