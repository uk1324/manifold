#include "Polyhedra.hpp"
#include <game/DoublyConnectedEdgeList.hpp>

FlatShadingResult flatShadeRegularPolyhedron(View<const Vec3> vertices, View<const i32> facesIndices, i32 verticesPerFace) {
	ASSERT(facesIndices.size() % verticesPerFace == 0);
	const auto faceCount = facesIndices.size() / verticesPerFace;
	ASSERT(verticesPerFace >= 3);

	FlatShadingResult result;

 	for (i32 faceI = 0; faceI < faceCount; faceI++) {
		const auto faceOffset = faceI * verticesPerFace;
		auto indexFace = [&](i32 indexInFace) {
			return facesIndices[faceOffset + indexInFace];
		};
		const auto normal = cross(
			vertices[indexFace(1)] - vertices[indexFace(0)],
			vertices[indexFace(2)] - vertices[indexFace(0)]
		).normalized();
		for (i32 vertexI = 0; vertexI < verticesPerFace; vertexI++) {
			const auto vertexIndex = indexFace(vertexI);
			result.positions.push_back(vertices[vertexIndex]);
			result.normals.push_back(normal);
		}
		f32 triangleFanCentralVertexIndexInFace = 0;
		for (i32 i = 0; i < verticesPerFace - 2; i++) {
			result.indices.push_back(faceOffset + triangleFanCentralVertexIndexInFace);
			result.indices.push_back(faceOffset + i + 1);
			result.indices.push_back(faceOffset + (i + 2) % verticesPerFace);
		}
	}
	return result;
}

FlatShadingResult flatShadeConvexPolygonSoup(View<const Vec3> vertices, View<const i32> facesIndices, View<const i32> verticesPerFace) {
	const auto faceCount = verticesPerFace.size();
	FlatShadingResult result;

	i32 faceOffset = 0;
	for (i32 faceI = 0; faceI < faceCount; faceI++) {
		auto indexFace = [&](i32 indexInFace) {
			return facesIndices[faceOffset + indexInFace];
		};
		const auto normal = cross(
			vertices[indexFace(1)] - vertices[indexFace(0)],
			vertices[indexFace(2)] - vertices[indexFace(0)]
		).normalized();

		const auto faceVertexCount = verticesPerFace[faceI];
		ASSERT(faceVertexCount >= 3);

		for (i32 vertexI = 0; vertexI < faceVertexCount; vertexI++) {
			const auto vertexIndex = indexFace(vertexI);
			result.positions.push_back(vertices[vertexIndex]);
			result.normals.push_back(normal);
		}
		// This is just a fan triangulation of a polygon.
		f32 triangleFanCentralVertexIndexInFace = 0;
		for (i32 i = 0; i < faceVertexCount - 2; i++) {
			result.indices.push_back(faceOffset + triangleFanCentralVertexIndexInFace);
			result.indices.push_back(faceOffset + i + 1);
			result.indices.push_back(faceOffset + (i + 2) % faceVertexCount);
		}
		faceOffset += faceVertexCount;
	}
	return result;
}

FlatShadingResult flatShadeConvexPolygonSoup(const PolygonSoup& polygonSoup) {
	return flatShadeConvexPolygonSoup(constView(polygonSoup.positions), constView(polygonSoup.facesVertices), constView(polygonSoup.verticesPerFace));
}

PolygonSoup regularPolyhedronPolygonSoup(View<const Vec3> vertices, View<const i32> facesIndices, i32 verticesPerFace){
	PolygonSoup result;
	for (const auto& vertex : vertices) {
		result.positions.push_back(vertex);
	}
	for (const auto& index : facesIndices) {
		result.facesVertices.push_back(index);
	}
	for (i32 i = 0; i < facesIndices.size() / verticesPerFace; i++) {
		result.verticesPerFace.push_back(verticesPerFace);
	}
	return result;
}

PolygonSoup dualPolyhedron(const PolygonSoup& polyhedron) {
	DoublyConnectedEdgeList mesh;
	mesh.initialize(constView(polyhedron.positions), constView(polyhedron.facesVertices), constView(polyhedron.verticesPerFace));

	PolygonSoup result;
	for (const auto& face : mesh.faces) {
		const auto centroid = mesh.computeFaceCentroid(face);
		result.positions.push_back(centroid);
	}
	for (const auto& vertex : mesh.vertices) {
		i32 count = 0;
		for (const auto& faceIndex : mesh.facesAroundVertex(vertex)) {
			result.facesVertices.push_back(faceIndex);
			count++;
		}
		result.verticesPerFace.push_back(count);
	}
	return result;
}
