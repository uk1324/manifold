#include "Polyhedra.hpp"
#include <game/DoublyConnectedEdgeList.hpp>
#include <engine/Math/Angles.hpp>

std::vector<Quat> cubeDirectIsometries() {
	std::vector<Quat> result;

	result.push_back(Quat::identity);

	{
		// axes though centers of faces
		Vec3 axes[]{
 			Vec3(1.0f, 0.0f, 0.0f),
			Vec3(0.0f, 1.0f, 0.0f),
			Vec3(0.0f, 0.0f, 1.0f),
		};
		for (const auto& axis : axes) {
			result.push_back(Quat(PI<f32> / 2.0f, axis));
			result.push_back(Quat(-PI<f32> / 2.0f, axis));
			result.push_back(Quat(PI<f32>, axis));
		}
	}
	{
		// axes though midpoints of edges
		Vec3 axes[]{
			Vec3(0.0f, 1.0f, 1.0f),
			Vec3(0.0f, -1.0f, 1.0f),
			Vec3(1.0f, 0.0f, 1.0f),
			Vec3(-1.0f, 0.0f, 1.0f),
			Vec3(1.0f, 1.0f, 0.0),
			Vec3(1.0f, -1.0f, 0.0),
		};
		for (const auto& axis : axes) {
			result.push_back(Quat(PI<f32>, axis.normalized()));
		}
	}
	{
		// axes though vertices
		Vec3 axes[]{
			// These are just the vertices of the top face
			Vec3(1.0f, 1.0f, 1.0f),
			Vec3(-1.0f, 1.0f, 1.0f),
			Vec3(-1.0f, -1.0f, 1.0f),
			Vec3(1.0f, -1.0f, 1.0f),
		};
		for (const auto& axis : axes) {
			result.push_back(Quat(TAU<f32> / 3.0f, axis.normalized()));
			result.push_back(Quat(-TAU<f32> / 3.0f, axis.normalized()));
		}
	}
	return result;
}

std::vector<Quat> icosahedronDirectIsometries() {
	// If we think of the isometries as acting on a dodecahedron then the isometries can be divided into
	// identity
	// rotation with an axis though a face center. Pairs of faces represent an axis. There are 12 faces => 6 axes. The order of rotation is 5 so there are 4 unique rotations. (12 / 2) * 4.
	// rotation with an axis though a vertex. There are 20 vertices => 10 axes. The oreder of rotation is 3 so there are 2 unique rotations. (20 / 2) * 2.
	// rotation with an axis though the midpoint of an edge. There are 30 edges => 15 axes. There order of rotation is 2 so there is one unique rotation. (30 / 2) * 1.

	// The vertices can be split into mirror images. Then one half of them will represent the axes.
	// The pairs of vertices represent an edge. Again for each pair there is an opposite pair.
	return std::vector<Quat>();
}

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
