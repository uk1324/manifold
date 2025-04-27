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

static const i32 cubeEdges[]{
    0, 3,
    0, 1,
    0, 4,
    1, 2,
    1, 5,
    2, 3,
    3, 7,
    2, 6,
    6, 7,
    4, 5,
    4, 7,
    5, 6,
};

/*
1---------0
 \  \ /  /
  \  3  /
   \ | /
    \|/ 
     2
*/
static const Vec3 tetrahedronVertices[]{
    // top
    Vec3(0.0f, 1.0f, sqrt(2.0f) / 2.0f),
    Vec3(0.0f, -1.0f, sqrt(2.0f) / 2.0f),
    // bottom
    Vec3(-1.0f, 0.0f, -sqrt(2.0f) / 2.0f),
    Vec3(1.0f, 0.0f, -sqrt(2.0f) / 2.0f),
};

// All the possible ways to choose 3 vertices from a set of 4.
static i32 tetrahedronFaces[]{
    0, 1, 2,
    1, 3, 2,
    0, 2, 3,
    0, 1, 3,
};

// All the possible ways to choose 2 vertices from a set of 4.
static i32 tetrahedronEdges[]{
    0, 1,
    0, 2,
    0, 3,
    2, 3,
    1, 2,
    1, 3
};

const auto phi = (1.0f + sqrt(5.0f)) / 2.0f;
static const Vec3 icosahedronVertices[]{
    Vec3(0.0f, 1.0f, phi),
    Vec3(0.0f, -1.0f, phi),
    Vec3(0.0f, 1.0f, -phi),
    Vec3(0.0f, -1.0f, -phi),
    Vec3(1.0f, phi, 0.0f),
    Vec3(-1.0f, phi, 0.0f),
    Vec3(1.0f, -phi, 0.0f),
    Vec3(-1.0f, -phi, 0.0f),
    Vec3(phi, 0.0f, 1.0f),
    Vec3(phi, 0.0f, -1.0f),
    Vec3(-phi, 0.0f, 1.0f),
    Vec3(-phi, 0.0f, -1.0f),
};

static i32 icosahedronFaces[]{
    8, 0, 1,
    10, 1, 0,
    4, 5, 0,
    7, 6, 1,
    11, 7, 10,
    11, 10, 5,
    9, 4, 8,
    9, 8, 6,
    2, 9, 3,
    2, 3, 11,
    2, 11, 5,
    2, 4, 9,
    3, 9, 6,
    3, 7, 11,
    4, 2, 5,
    3, 6, 7,
    6, 8, 1,
    7, 1, 10,
    5, 10, 0,
    4, 0, 8
};

static const i32 icosahedronEdges[]{
    2, 4,
    2, 5,
    1, 8,
    1, 10,
    2, 3,
    1, 6,
    1, 7,
    0, 8,
    0, 10,
    0, 1,
    0, 4,
    0, 5,
    3, 11,
    3, 9,
    4, 5,
    5, 10,
    5, 11,
    6, 7,
    10, 11,
    7, 10,
    7, 11,
    8, 9,
    4, 8,
    4, 9,
    6, 8,
    6, 9,
    3, 7,
    3, 6,
    2, 11,
    2, 9,
};

static i32 icosahedronVerticesPerFace = 3;
static i32 icosahedronFaceCount = std::size(icosahedronFaces) / icosahedronVerticesPerFace;

// direct and opposite isometries
// proper and improper isometries.
std::vector<Quat> generateOctahedralDirectSymmetriesQuats();
std::vector<Quat> generateTetrahedralDirectSymmetriesQuats();
// The issue with these is that they don't generate the symmetries of the icosahedron model above.
// Actually it was probably some bug somewhere else in the code.
std::vector<Mat3> generateIcosahedralDirectSymmetriesMats();
std::vector<Quat> generateIcosahedralDirectSymmetriesQuats();
std::vector<Quat> generateIcosahedralDirectSymmetriesQuats2();

static const Quat tetrahedralSymmetries[]{
    Quat(0.0f, 0.0f, 0.0f, 1.0f),
    Quat(0.0f, 0.0f, 1.0f, -4.37114e-08f),
    Quat(-0.707107f, 0.707107f, 0.0f, -4.37114e-08f),
    Quat(-0.707107f, -0.707107f, 0.0f, -4.37114e-08f),
    Quat(0.0f, 0.707107f, 0.5f, 0.5f),
    Quat(-0.0f, -0.707107f, -0.5f, 0.5f),
    Quat(0.0f, -0.707107f, 0.5f, 0.5f),
    Quat(-0.0f, 0.707107f, -0.5f, 0.5f),
    Quat(-0.707107f, 0.0f, -0.5f, 0.5f),
    Quat(0.707107f, -0.0f, 0.5f, 0.5f),
    Quat(0.707107f, 0.0f, -0.5f, 0.5f),
    Quat(-0.707107f, -0.0f, 0.5f, 0.5f),
};

static const Quat octahedralSymmetries[]{
    Quat(0.0f, 0.0f, 0.0f, 1.0f),
    Quat(0.707107f, 0.0f, 0.0f, 0.707107f),
    Quat(-0.707107f, -0.0f, -0.0f, 0.707107f),
    Quat(1.0f, 0.0f, 0.0f, -4.37114e-08f),
    Quat(0.0f, 0.707107f, 0.0f, 0.707107f),
    Quat(-0.0f, -0.707107f, -0.0f, 0.707107f),
    Quat(0.0f, 1.0f, 0.0f, -4.37114e-08f),
    Quat(0.0f, 0.0f, 0.707107f, 0.707107f),
    Quat(-0.0f, -0.0f, -0.707107f, 0.707107f),
    Quat(0.0f, 0.0f, 1.0f, -4.37114e-08f),
    Quat(0.0f, 0.707107f, 0.707107f, -4.37114e-08f),
    Quat(0.0f, -0.707107f, 0.707107f, -4.37114e-08f),
    Quat(0.707107f, 0.0f, 0.707107f, -4.37114e-08f),
    Quat(-0.707107f, 0.0f, 0.707107f, -4.37114e-08f),
    Quat(0.707107f, 0.707107f, 0.0f, -4.37114e-08f),
    Quat(0.707107f, -0.707107f, 0.0f, -4.37114e-08f),
    Quat(0.5f, 0.5f, 0.5f, 0.5f),
    Quat(-0.5f, -0.5f, -0.5f, 0.5f),
    Quat(-0.5f, 0.5f, 0.5f, 0.5f),
    Quat(0.5f, -0.5f, -0.5f, 0.5f),
    Quat(-0.5f, -0.5f, 0.5f, 0.5f),
    Quat(0.5f, 0.5f, -0.5f, 0.5f),
    Quat(0.5f, -0.5f, 0.5f, 0.5f),
    Quat(-0.5f, 0.5f, -0.5f, 0.5f),
};

static Quat icosahedralSymmetries[]{
    Quat(0.0f, 0.0f, 0.0f, 1.0f),
    Quat(0.309017f, 0.0f, 0.809017f, 0.5f),
    Quat(-0.309017f, -0.0f, -0.809017f, 0.5f),
    Quat(-0.309017f, 0.0f, 0.809017f, 0.5f),
    Quat(0.309017f, -0.0f, -0.809017f, 0.5f),
    Quat(0.0f, 0.809017f, 0.309017f, 0.5f),
    Quat(-0.0f, -0.809017f, -0.309017f, 0.5f),
    Quat(0.0f, -0.809017f, 0.309017f, 0.5f),
    Quat(-0.0f, 0.809017f, -0.309017f, 0.5f),
    Quat(-0.809017f, -0.309017f, 0.0f, 0.5f),
    Quat(0.809017f, 0.309017f, -0.0f, 0.5f),
    Quat(-0.809017f, 0.309017f, 0.0f, 0.5f),
    Quat(0.809017f, -0.309017f, -0.0f, 0.5f),
    Quat(-0.5f, 0.5f, -0.5f, 0.5f),
    Quat(0.5f, -0.5f, 0.5f, 0.5f),
    Quat(0.5f, 0.5f, -0.5f, 0.5f),
    Quat(-0.5f, -0.5f, 0.5f, 0.5f),
    Quat(0.5f, -0.5f, -0.5f, 0.5f),
    Quat(-0.5f, 0.5f, 0.5f, 0.5f),
    Quat(-0.5f, -0.5f, -0.5f, 0.5f),
    Quat(0.5f, 0.5f, 0.5f, 0.5f),
    Quat(0.309017f, 0.809017f, -0.5f, -4.37114e-08f),
    Quat(-0.309017f, 0.809017f, -0.5f, -4.37114e-08f),
    Quat(0.5f, -0.309017f, 0.809017f, -4.37114e-08f),
    Quat(-0.5f, -0.309017f, 0.809017f, -4.37114e-08f),
    Quat(0.0f, 0.0f, -1.0f, -4.37114e-08f),
    Quat(0.5f, 0.309017f, 0.809017f, -4.37114e-08f),
    Quat(-0.5f, 0.309017f, 0.809017f, -4.37114e-08f),
    Quat(0.309017f, 0.809017f, 0.5f, -4.37114e-08f),
    Quat(-0.309017f, 0.809017f, 0.5f, -4.37114e-08f),
    Quat(0.0f, 1.0f, 0.0f, -4.37114e-08f),
    Quat(-0.809017f, 0.5f, 0.309017f, -4.37114e-08f),
    Quat(-0.809017f, 0.5f, -0.309017f, -4.37114e-08f),
    Quat(-1.0f, 0.0f, 0.0f, -4.37114e-08f),
    Quat(-0.809017f, -0.5f, 0.309017f, -4.37114e-08f),
    Quat(-0.809017f, -0.5f, -0.309017f, -4.37114e-08f),
    Quat(0.0f, 0.309017f, 0.5f, 0.809017f),
    Quat(0.0f, 0.5f, 0.809017f, 0.309017f),
    Quat(-0.0f, -0.309017f, -0.5f, 0.809017f),
    Quat(-0.0f, -0.5f, -0.809017f, 0.309017f),
    Quat(0.0f, -0.309017f, 0.5f, 0.809017f),
    Quat(0.0f, -0.5f, 0.809017f, 0.309017f),
    Quat(-0.0f, 0.309017f, -0.5f, 0.809017f),
    Quat(-0.0f, 0.5f, -0.809017f, 0.309017f),
    Quat(0.309017f, 0.5f, 0.0f, 0.809017f),
    Quat(0.5f, 0.809017f, 0.0f, 0.309017f),
    Quat(-0.309017f, -0.5f, -0.0f, 0.809017f),
    Quat(-0.5f, -0.809017f, -0.0f, 0.309017f),
    Quat(-0.309017f, 0.5f, 0.0f, 0.809017f),
    Quat(-0.5f, 0.809017f, 0.0f, 0.309017f),
    Quat(0.309017f, -0.5f, -0.0f, 0.809017f),
    Quat(0.5f, -0.809017f, -0.0f, 0.309017f),
    Quat(0.5f, 0.0f, 0.309017f, 0.809017f),
    Quat(0.809017f, 0.0f, 0.5f, 0.309017f),
    Quat(-0.5f, -0.0f, -0.309017f, 0.809017f),
    Quat(-0.809017f, -0.0f, -0.5f, 0.309017f),
    Quat(0.5f, 0.0f, -0.309017f, 0.809017f),
    Quat(0.809017f, 0.0f, -0.5f, 0.309017f),
    Quat(-0.5f, -0.0f, 0.309017f, 0.809017f),
    Quat(-0.809017f, -0.0f, 0.5f, 0.309017f),
};

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

void edgeListFromFaceList(View<const i32> faces, View<const i32> verticesPerFace);

struct MeshTriPn {
    std::vector<Vec3> positions;
    std::vector<Vec3> normals;
    std::vector<i32> indices;
};

MeshTriPn makeIcosphere(i32 edgeDivisions, f32 radius);