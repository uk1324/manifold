#pragma once

#include <vector>
#include <engine/Math/Vec3.hpp>
#include <engine/Math/Vec2.hpp>

struct SurfaceData {
	std::vector<Vec3> positions;
	std::vector<Vec3> normals;
	std::vector<Vec2> uvs;
	// These always go from 0 to 1, could compute then on the fly instead of precomputing.
	std::vector<Vec2> uvts;
	std::vector<i32> indices;
	std::vector<f32> curvatures;
	f32 minCurvature;
	f32 maxCurvature;

	std::vector<Vec3> triangleCenters;
	std::vector<f32> triangleAreas;
	f32 totalArea;

	std::vector<i32> sortedTriangles;
	// Instead of using triangles could make a triangle fan or triangle strip.
	void sortTriangles(Vec3 cameraPosition);

	i32 vertexCount() const;
	i32 triangleCount() const;
	void addVertex(Vec3 p, Vec3 n, Vec2 uv, Vec2 uvt);
};
