#include "Polyhedra.hpp"
#include <game/DoublyConnectedEdgeList.hpp>
#include <engine/Math/Angles.hpp>
#include <unordered_map>
#include "Permutations.hpp"
#include <Put.hpp>
#include <unordered_set>
#include <iomanip>

void printQuaternions(std::vector<Quat> qs) {
	put("Quat result[] {");
	for (const auto& q : qs) {
		put("\tQuat(%f, %f, %f, %f),", q.x, q.y, q.z, q.w);
	}
	std::cout << std::setprecision(std::numeric_limits<f32>::digits10);
	put("}");
}

std::vector<Quat> generateOctahedralDirectSymmetriesQuats() {
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


Quat mat3ToQuatUnchecked(const Mat3& ma) {
	//auto m = ma.transposed();
	auto m = ma;
	Quat result = Quat::identity;
	f32 t = 1.0f;
	if (m(2, 2) < 0) {
		if (m(0, 0) > m(1, 1)) {
			t = 1 + m(0, 0) - m(1, 1) - m(2, 2);
			result = Quat(t, m(0, 1) + m(1, 0), m(2, 0) + m(0, 2), m(1, 2) - m(2, 1));
		} else {
			t = 1 - m(0, 0) + m(1, 1) - m(2, 2);
			result = Quat(m(0, 1) + m(1, 0), t, m(1, 2) + m(2, 1), m(2, 0) - m(0, 2));
		}
	} else {
		if (m(0, 0) < -m(1, 1)) {
			t = 1 - m(0, 0) - m(1, 1) + m(2, 2);
			result = Quat(m(2, 0) + m(0, 2), m(1, 2) + m(2, 1), t, m(0, 1) - m(1, 0));
		} else {
			t = 1 + m(0, 0) + m(1, 1) + m(2, 2);
			result = Quat(m(1, 2) - m(2, 1), m(2, 0) - m(0, 2), m(0, 1) - m(1, 0), t);
		}
	}
	result *= 0.5 / sqrt(t);
	return result.normalized();
}

std::vector<Quat> generateTetrahedralDirectSymmetriesQuats() {
	std::vector<Quat> result;
	result.push_back(Quat::identity);
	{
		const Vec3 axes[]{
			(tetrahedronVertices[0] + tetrahedronVertices[1]) / 2.0f,
			(tetrahedronVertices[0] + tetrahedronVertices[2]) / 2.0f,
			(tetrahedronVertices[1] + tetrahedronVertices[2]) / 2.0f,
		};
		for (const auto& axis : axes) {
			result.push_back(Quat(TAU<f32> / 2.0f, axis.normalized()));
		}
	}
	{
		for (const auto& vertex : tetrahedronVertices) {
			const auto axis = vertex.normalized();
			result.push_back(Quat(TAU<f32> / 3.0f, axis));
			result.push_back(Quat(-TAU<f32> / 3.0f, axis));
		}
	}
	return result;
}

std::vector<Mat3> generateIcosahedralDirectSymmetriesMats() {
	// If we think of the isometries as acting on a dodecahedron then the isometries can be divided into
	// identity
	// rotation with an axis though a face center. Pairs of faces represent an axis. There are 12 faces => 6 axes. The order of rotation is 5 so there are 4 unique rotations. (12 / 2) * 4.
	// rotation with an axis though a vertex. There are 20 vertices => 10 axes. The oreder of rotation is 3 so there are 2 unique rotations. (20 / 2) * 2.
	// rotation with an axis though the midpoint of an edge. There are 30 edges => 15 axes. There order of rotation is 2 so there is one unique rotation. (30 / 2) * 1.
	// These groups are disjoint, because each group only contains elements of one order and each group contains elements of different orders.
	// All of these are unique, because they have different orders. The orders of powers of an element are divisors of the order of that element. So for order 5 the divisors are 5, 1, for 3 its 3, 1 and for 2 its 2, 1
	// This method generalizes to the symmetry groups of other polyhedra.

	// The vertices can be split into mirror images. Then one half of them will represent the axes.
	// The pairs of vertices represent an edge. Again for each pair there is an opposite pair.	
	

	// I just found the A_5 generators here:
	// https://math.stackexchange.com/questions/1999312/group-presentation-of-a-5-with-two-generators
	// And took the coresponding matricies from here:
	// https://en.wikipedia.org/wiki/Icosahedral_symmetry#Isomorphism_of_I_with_A5
	std::vector<Permutation> generators{
		Permutation::fromOneIndexed({ 5, 1, 2, 3, 4 }), // (1 2 3 4 5)
		Permutation::fromOneIndexed({ 2, 1, 4, 3, 5 }), // (1 2)(3 4)
	};
	const auto p = (sqrt(5.0f) + 1.0f) / 2.0f;
	std::vector<Mat3> generatorsQuaternions {
		Mat3{
			-1.0f / (2.0f * p), p / 2.0f, -0.5f,
			p / 2.0f, 0.5f, 1.0f / (2.0f * p), 
			0.5f, -1.0f / (2.0f * p), -p / 2.0f
		},
		Mat3{
			-0.5f, 1.0f / (2.0f * p), p / 2.0f,
			1 / (2.0f * p), -p / 2.0f, 0.5f,
			p / 2.0f, 0.5f, 1 / (2.0f * p)
		}
	};
	// To generate all the elements this has to go though word up to length 10.
	// This means that it checks
	// 3^10 + 3^9 + 3^8 + 3^7 + 3^6 + 3^5 + 3^4 + 3^3 + 3^2 + 3^1 = 88572
	// words
	const auto groupSize = 60;
	const auto generatedElements = generatePermutationGroup(generators, groupSize);


	std::vector<Mat3> r;

	for (const auto& [permutation, word] : generatedElements) {
		if (word.size() == 0) {
			r.push_back(Mat3::identity);
			continue;
		}
		auto generatedQuat = generatorsQuaternions[word[0]];
		for (i32 i = 1; i < word.size(); i++) {
			generatedQuat = generatorsQuaternions[word[i]] * generatedQuat;
		}
		r.push_back(generatedQuat);
	}
 	return r;
}

std::vector<Quat> generateIcosahedralDirectSymmetriesQuats() {
	const auto mats = generateIcosahedralDirectSymmetriesMats();
	std::vector<Quat> result;
	for (const auto& mat : mats) {
		result.push_back(mat3ToQuatUnchecked(mat));
	}
	printQuaternions(result);
	return result;
}

template<typename T, typename Eq>
void vectorAddIfUnique(std::vector<T>& v, const T& toAdd, Eq equals) {
	bool duplicate = false;
	for (const auto& item : v) {
		if (equals(item, toAdd)) {
			duplicate = true;
			break;
		}
	}
	if (!duplicate) {
		v.push_back(toAdd);
	}
}

std::vector<Quat> generateIcosahedralDirectSymmetriesQuats2() {
	std::vector<Quat> result;
	result.push_back(Quat::identity);

	auto equals = [](View<const Vec3> a, View<const Vec3> b) {
		for (const auto& va : a) {
			bool found = false;
			for (const auto& ba : b) {
				if (va == -ba) {
					found = true;
					break;
				}
			}

			if (!found) {
				return false;
			}
		}
		return true;
	};

	{
		struct Face {
			Vec3 v[3];
		};
		auto faceEquals = [&](const Face& a, const Face& b) {
			return equals(constView(a.v), constView(b.v));
		};

		std::vector<Face> uniqueFacesModAntipodalMap;
		for (i32 i = 0; i < std::size(icosahedronFaces); i += icosahedronVerticesPerFace) {
			const auto newFace = Face{ .v = {
				icosahedronVertices[icosahedronFaces[i]],
				icosahedronVertices[icosahedronFaces[i + 1]],
				icosahedronVertices[icosahedronFaces[i + 2]]
			} };
			vectorAddIfUnique(uniqueFacesModAntipodalMap, newFace, faceEquals);
		}

		std::vector<Vec3> axes;
		for (const auto& face : uniqueFacesModAntipodalMap) {
			const auto center = (face.v[0] + face.v[1] + face.v[2]) / 3.0f;
			axes.push_back(center.normalized());
		}
		for (const auto& axis : axes) {
			result.push_back(Quat(TAU<f32> / 3.0f, axis));
			result.push_back(Quat(-TAU<f32> / 3.0f, axis));
		}
	}
	{
		struct Edge {
			Vec3 v[2];
		};
		auto edgeEquals = [&](const Edge& a, const Edge& b) {
			return equals(constView(a.v), constView(b.v));
		};
		std::vector<Edge> uniqueEdgesModAntipodalMap;
		for (i32 i = 0; i < std::size(icosahedronEdges); i += 2) {
			const auto newEdge = Edge{ .v = {
				icosahedronVertices[icosahedronEdges[i]],
				icosahedronVertices[icosahedronEdges[i + 1]],
			} };
			vectorAddIfUnique(uniqueEdgesModAntipodalMap, newEdge, edgeEquals);
		}

		std::vector<Vec3> axes;
		for (const auto& edge : uniqueEdgesModAntipodalMap) {
			const auto center = (edge.v[0] + edge.v[1]) / 2.0f;
			axes.push_back(center.normalized());
		}
		for (const auto& axis : axes) {
			result.push_back(Quat(PI<f32>, axis));
		}
	}
	{
		std::vector<Vec3> uniqueVerticesModAntipodalMap;
		auto vertexEquals = [&](const Vec3& a, const Vec3& b) {
			return a == -b;
		};
		for (const auto& vertex : icosahedronVertices) {
			vectorAddIfUnique(uniqueVerticesModAntipodalMap, vertex, vertexEquals);
		}

		for (const auto& vertex : uniqueVerticesModAntipodalMap) {
			const auto axis = vertex.normalized();
			result.push_back(Quat(TAU<f32> / 5.0f, axis));
			result.push_back(Quat(2.0f * TAU<f32> / 5.0f, axis));
			result.push_back(Quat(-TAU<f32> / 5.0f, axis));
			result.push_back(Quat(-2.0f * TAU<f32> / 5.0f, axis));
		}
	}
	return result;
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

// "Introduction to geometry" page 273
// If you have a tetrahedron then a reflection in any plane going though an edge and the midpoint of the opposite edge swaps 2 faces. These are transpositions on the set of all faces so that each of these can be identified with a permutation of faces. Further consideration shows that by identifiying the reflection with the transpostion you can create an isomorphism from the tetrahedral group to S_4. The even permutations correspond to proper isometries and the odd permuations to improper ones so that the alternating group A_4 is isomorphic to the group of proper symmetries of a teterahedron.
// Smilarly coloring faces of the other polyhedra (sometimes multiples faces with the same color) can be used to give an isomorphism between:
// the group of proper symmetries of a cube and a tetrahedron is S_4
// the group of all symmetries of an icosahedron and a dodecahedron is S_5, and of the proper symmetries is A_5.

// How did someone find these colorings? Are they unique? How could you come up with them on the spot? It is mentioned that the colorings have no vertex edge or face in common, but this isn't enough, because you can still swap things that break the general sturcture, but still presevere the no vertex edge or face in common condition.
// I guess you could look at the orders of elements.


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

void edgeListFromFaceList(View<const i32> faces, View<const i32> verticesPerFace) {
	using EdgeId = std::pair<i32, i32>;
	auto makeEdgeId = [](i32 a, i32 b) -> EdgeId {
		if (a < b) {
			return EdgeId{ a, b };
		}
		return EdgeId{ b, a };
	};
	struct EdgeIdHash {
		usize operator()(const EdgeId& id) const {
			return hashCombine(id.first, id.second);
		}
	};
	std::unordered_set<EdgeId, EdgeIdHash> edgeList;
	i32 offset = 0;
	for (i32 faceI = 0; faceI < verticesPerFace.size(); faceI++) {
		const auto verticesInFace = verticesPerFace[faceI];
		i32 previousVertex = verticesInFace - 1;
		for (i32 vertexI = 0; vertexI < verticesInFace; vertexI++) {
			edgeList.insert(makeEdgeId(faces[offset + vertexI], faces[offset + previousVertex]));
			previousVertex = vertexI;
		}
		offset += verticesInFace;
	}
	put("i32 edges[]{");
	for (const auto& edge : edgeList) {
		put("\t%, %,", edge.first, edge.second);
	}
	put("};");
}
