#pragma once

#include "Surfaces/Torus.hpp"
#include <vector>

struct Surface {
	enum class Type {
		TORUS,
	};

	Torus torus{ .r = 0.4f, .R = 1.0f };
	//Sphere sphere{ .r = 1.0f };

	void initialize(Type selected);

	Vec3 position(Vec2 uv) const;
	Vec3 normal(Vec2 uv) const;
	Vec3 tangentU(Vec2 uv) const;
	Vec3 tangentV(Vec2 uv) const;
	ChristoffelSymbols christoffelSymbols(Vec2 uv) const;

	f32 uMin() const;
	f32 uMax() const;
	f32 vMin() const;
	f32 vMax() const;
	SquareSideConnectivity uConnectivity() const;
	SquareSideConnectivity vConnectivity() const;

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

	void sortTriangles(Vec3 cameraPosition);

	i32 vertexCount() const;
	i32 triangleCount() const;
	void addVertex(Vec3 p, Vec3 n, Vec2 uv, Vec2 uvt);

	Type selected = Type::TORUS;
};
