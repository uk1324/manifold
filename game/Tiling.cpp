#include "Tiling.hpp"
#include <engine/Math/GramSchmidt.hpp>
#include <game/Math.hpp>

// https://stackoverflow.com/questions/46770028/is-it-possible-to-use-stdset-intersection-to-check-if-two-sets-have-any-elem
template <class I1, class I2>
bool haveCommonElement(I1 first1, I1 last1, I2 first2, I2 last2) {
	while (first1 != last1 && first2 != last2) {
		if (*first1 < *first2)
			++first1;
		else if (*first2 < *first1)
			++first2;
		else
			return true;
	}
	return false;
}

Tiling::Tiling(const Polytope& c) {
	auto outwardPointingFaceNormal = [&](const std::vector<Vec4>& vertices, const std::vector<Face>& faces, const std::vector<i32>& cellFaces, i32 faceI) {
		const auto& face = faces[faceI];

		auto normal = crossProduct(
			vertices[face.vertices[0]],
			vertices[face.vertices[1]],
			vertices[face.vertices[2]]
		).normalized();
		// The normal should point outward of the cell, that is every vertex of the cell not belonging to the face shuld have a negative dot product with the normal, the code below negates the normal if that is not the case. This is analogous to the case of a sphere. If we have 2 vertices on the sphere we can take their cross product to get the normal of the plane that intersects those 2 vertices. 
		for (const auto& someOtherFaceI : cellFaces) {
			if (someOtherFaceI == faceI) {
				continue;
			}
			const auto& someOtherFace = faces[someOtherFaceI];
			for (const auto& someOtherFaceVertexI : someOtherFace.vertices) {
				bool foundVertexNotBelongingToFace = true;
				for (const auto& faceVertexI : face.vertices) {
					if (someOtherFaceVertexI == faceVertexI) {
						foundVertexNotBelongingToFace = false;
						break;
					}
				}
				if (foundVertexNotBelongingToFace) {
					if (dot(vertices[someOtherFaceVertexI], normal) > 0.0f) {
						normal = -normal;
					}
					return normal;
				}
			}
		}
		CHECK_NOT_REACHED();
		return normal;
	};

	for (i32 i = 0; i < c.vertices.size(); i++) {
		const auto& vertex = c.vertices[i];
		vertices.push_back(Vec4(vertex[0], vertex[1], vertex[2], vertex[3]).normalized());
	}
	for (const auto& edge : c.cellsOfDimension(1)) {
		edges.push_back(Edge{ edge[0], edge[1] });
	}
	for (const auto& face : c.cellsOfDimension(2)) {
		auto sortedEdges = faceEdgesSorted(c, i32(&face - c.cellsOfDimension(2).data()));

		Face f{ 
			.vertices = verticesOfFaceWithSortedEdges(c, sortedEdges)
		};

		Vec4 orthonormalBasisFor3SpaceContainingPolygon[]{
			vertices[f.vertices[0]], 
			vertices[f.vertices[1]], 
			vertices[f.vertices[2]]
		};
		gramSchmidtOrthonormalize(::view(orthonormalBasisFor3SpaceContainingPolygon));

		auto planeThoughPoints = [&](Vec4 p0, Vec4 p1) {
			/*
			If you have a 2-sphere then you can represent lines on it by spheres that intersect the sphere in them. This is the same idea, but we first intersect the 3-sphere with a 3-space to get a sphere and then do the same thing. Then you can check if a point lies on an face using dot products with edges.
			*/
			/*
			Could maybe do this by just using the cross product of e0, e1, and the 3d plane normal, but then I would need to figure out the correct orientation some other way. Thinking about it now I don't really know why it works. That is why the edge normals point in the right direction. It might be accidiental, because the orthonormal basis might not be correctly oriented always.
			*/
			const auto e0 = coordinatesInOrthonormal3Basis(orthonormalBasisFor3SpaceContainingPolygon, p0);
			const auto e1 = coordinatesInOrthonormal3Basis(orthonormalBasisFor3SpaceContainingPolygon, p1);
			const auto plane2Normal = cross(e0, e1);
			const auto normalIn4Space = linearCombination(orthonormalBasisFor3SpaceContainingPolygon, plane2Normal);
			return normalIn4Space;
			};

		i32 previousI = i32(f.vertices.size()) - 1;
		for (i32 i = 0; i < f.vertices.size(); i++) {
			f.edgeNormals.push_back(planeThoughPoints(
				vertices[f.vertices[previousI]],
				vertices[f.vertices[i]])
			);
			previousI = i;
		}

		const auto fanBaseVertexI = f.vertices[0];
		const auto& fanBaseVertex = vertices[fanBaseVertexI];
		for (i32 i = 1; i < i32(f.vertices.size()) - 1; i++) {
			const auto i0 = f.vertices[i];
			const auto i1 = f.vertices[i + 1];
			Vec4 v0 = vertices[i0];
			Vec4 v1 = vertices[i1];
			Triangle triangle{
				.vertices = { fanBaseVertexI, i0, i1 },
				.edgeNormals = {
					planeThoughPoints(fanBaseVertex, v0),
					planeThoughPoints(v0, v1),
					planeThoughPoints(v1, fanBaseVertex),
				}
			};
			f.triangulation.push_back(std::move(triangle));
		}

		faces.push_back(f);
	}
	for (const auto& cell : c.cellsOfDimension(3)) {
		const auto currentCellI = i32(cells.size());

		std::vector<Vec4> faceNormals;
		for (i32 faceI : cell) {
			const auto normal = outwardPointingFaceNormal(vertices, faces, cell, faceI);
			faceNormals.push_back(normal);
		}

		cells.push_back(Cell{
			.faces = cell,
			.faceNormals = std::move(faceNormals),
			.centroid = Vec4(0.0f)
		});
	}
	{
		std::vector<StaticList<i32, 2>> faceToCells;
		faceToCells.resize(faces.size());

		for (i32 cellI = 0; cellI < cells.size(); cellI++) {
			auto& cell = cells[cellI];
			for (const auto& faceI : cell.faces) {
				auto& faceCells = faceToCells[faceI];
				if (faceCells.size() >= 2) {
					ASSERT_NOT_REACHED();
					break;
				}
				faceCells.add(cellI);
			}
		}

		for (i32 faceI = 0; faceI < faces.size(); faceI++) {
			auto& cells = faceToCells[faceI];
			ASSERT(cells.size() <= 2);
			for (i32 i = 0; i < 2; i++) {
				faces[faceI].cells.add(cells[i]);
				//r.faces[faceI].cells[i] = cells[i];
			}
		}
	}

	for (i32 cellI = 0; cellI < cells.size(); cellI++) {
		auto& cell = cells[cellI];
		/*for (const auto& faceI : cell.faces) {
			auto& face = r.faces[faceI];
			if (face.cells[0] == cellI) {
				cell.neighbouringCells.push_back(face.cells[1]);
			} else {
				cell.neighbouringCells.push_back(face.cells[0]);
			}
		}*/

		std::set<i32> cellVertices;
		for (const auto& faceI : cell.faces) {
			for (const auto& vertex : faces[faceI].vertices) {
				cellVertices.insert(vertex);
			}
		}
		cell.vertices = std::move(cellVertices);
		//r.cellsVertices.push_back(std::move(vertices));

		Vec4 centroid(0.0f);
		for (const auto& vertex : cell.vertices) {
			centroid += vertices[vertex];
		}
		centroid /= cell.vertices.size();
		cell.centroid = centroid.normalized();
	}

	
}

std::vector<std::vector<i32>> Tiling::cellsNeighbouringToCell() {
	std::vector<std::vector<i32>> r;
	r.resize(cells.size());
	for (i32 cellI = 0; cellI < cells.size(); cellI++) {
		for (i32 cellJ = cellI + 1; cellJ < cells.size(); cellJ++) {
			auto& vI = cells[cellI].vertices;
			auto& vJ = cells[cellJ].vertices;
			if (haveCommonElement(vI.begin(), vI.end(), vJ.begin(), vJ.end())) {
				r[cellI].push_back(cellJ);
				r[cellJ].push_back(cellI);
			}
		}
	}
	return r;
}
