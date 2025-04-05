#pragma once

#include <engine/Math/Vec3.hpp>
#include <View.hpp>
#include <engine/Math/Quat.hpp>
/*
  1----0
 /|   /|
2----3 |
| 5--|-4
|/   |/
6----7
*/
// 2, 1, 5, 6
static Vec3 cubeVertices[]{
	// Top face
	// 1-0
	// | |
	// 2-3
	Vec3(1.0f, 1.0f, 1.0f), // 0
	Vec3(-1.0f, 1.0f, 1.0f), // 1
	Vec3(-1.0f, -1.0f, 1.0f), // 2
	Vec3(1.0f, -1.0f, 1.0f), // 3
	// Bottom face
	// 5-4
	// | |
	// 6-7
	Vec3(1.0f, 1.0f, -1.0f), // 4
	Vec3(-1.0f, 1.0f, -1.0f), // 5
	Vec3(-1.0f, -1.0f, -1.0f), // 6
	Vec3(1.0f, -1.0f, -1.0f), // 7
};

static i32 cubeVerticesPerFace = 4;

static i32 cubeFaces[]{
	0, 1, 2, 3, // top 0
	3, 2, 6, 7, // front 1
	4, 0, 3, 7, // right 2 
	2, 1, 5, 6, // left 3 
	4, 7, 6, 5, // bottom 4
	1, 0, 4, 5, // back 5
};

// direct and opposite isometries
// proper and improper isometries.
std::vector<Quat> generateCubeDirectSymmetriesQuats();
std::vector<Mat3> generateIcosahedronDirectSymmetriesMats();
std::vector<Quat> generateIcosahedronDirectSymmetriesQuats();

struct PolygonSoup {
	std::vector<Vec3> positions;
	std::vector<i32> facesVertices;
	std::vector<i32> verticesPerFace;
};

#include <vector>
struct FlatShadingResult {
	std::vector<Vec3> positions;
	std::vector<Vec3> normals;
	std::vector<i32> indices;
};

FlatShadingResult flatShadeRegularPolyhedron(View<const Vec3> vertices, View<const i32> facesIndices, i32 verticesPerFace);
// Soup of planar convex polygons.
FlatShadingResult flatShadeConvexPolygonSoup(View<const Vec3> vertices, View<const i32> facesIndices, View<const i32> verticesPerFace);
FlatShadingResult flatShadeConvexPolygonSoup(const PolygonSoup& polygonSoup);

PolygonSoup regularPolyhedronPolygonSoup(View<const Vec3> vertices, View<const i32> facesIndices, i32 verticesPerFace);

PolygonSoup dualPolyhedron(const PolygonSoup& polyhedron);

