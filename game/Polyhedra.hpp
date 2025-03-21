#pragma once

#include <engine/Math/Vec3.hpp>
#include <View.hpp>
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
	0, 1, 2, 3, // top
	3, 2, 6, 7, // front
	4, 0, 3, 7, // right
	2, 1, 5, 6, // left
	4, 5, 6, 7, // bottom
	1, 0, 4, 5, // back
};

#include <vector>
struct FlatShadingResult {
	std::vector<Vec3> positions;
	std::vector<Vec3> normals;
	std::vector<i32> indices;
};
FlatShadingResult flatShadeRegularPolyhedron(View<const Vec3> vertices, View<const i32> facesIndices, i32 verticesPerFace);
