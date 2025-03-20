#pragma once

#include "Surfaces/Torus.hpp"
#include <game/Surfaces/KleinBottle.hpp>
#include <random>
#include <vector>

struct SurfacePosition {
	static SurfacePosition makeUv(f32 u, f32 v);
	static SurfacePosition makeUv(Vec2 uv);

	SurfacePosition();
	explicit SurfacePosition(Vec2 uv);

	union {
		Vec2 uv;
		Vec3 sphere;
	};
};

struct SurfaceTangent {
	SurfaceTangent();
	static SurfaceTangent makeUv(Vec2 uv);
	static SurfaceTangent zero();
	explicit SurfaceTangent(Vec2 uv);

	Vec2 uv;
};

// Tangent space direction.
struct SurfaceDirection {
	f32 uvAngle;
	Vec2 direction;
	Vec3 sphereDirection;
};

struct Surface {
	enum class Type {
		TORUS,
		KLEIN_BOTTLE,
	};

	//Torus torus{ .r = 0.4f, .R = 1.0f };
	//Torus torus{ .r = 0.4f, .R = 0.7f };
	Torus torus{ .r = 0.3f, .R = 0.55f };
	KleinBottle kleinBottle;
	//Torus torus{ .r = 1.0f, .R = 2.5f };
	//Torus torus{ .r = 0.7f, .R = 1.5f };
	//Sphere sphere{ .r = 1.0f };

	void initialize(Type selected);

	SurfaceTangent scaleTangent(SurfaceTangent tangent, f32 scale) const;

	Vec3 position(SurfacePosition pos) const;
	Vec3 normal(SurfacePosition pos) const;
	Vec3 tangentU(SurfacePosition pos) const;
	Vec3 tangentV(SurfacePosition pos) const;
	ChristoffelSymbols christoffelSymbols(SurfacePosition pos) const;

	SurfacePosition randomPointOnSurface();
	SurfaceTangent randomTangentVectorAt(SurfacePosition position, f32 length);

	SurfaceTangent tangentVectorFromPolar(SurfacePosition position, f32 angle, f32 length);
	SurfaceTangent tangentVectorNormalize(SurfacePosition position, SurfaceTangent tangent);

	f32 uMin() const;
	f32 uMax() const;
	f32 vMin() const;
	f32 vMax() const;
	SquareSideConnectivity uConnectivity() const;
	SquareSideConnectivity vConnectivity() const;

	void integrateParticle(SurfacePosition& position, SurfaceTangent& velocity);
	SurfacePosition moveForward(SurfacePosition position, SurfaceTangent direction, f32 distance);
	struct MoveForwardResult {
		SurfacePosition position;
		SurfaceTangent finalDirection;
	};
	MoveForwardResult moveForwardAndReturnDirection(SurfacePosition position, SurfaceTangent direction, f32 distance);
	//SurfacePosition addVelocity(SurfacePosition)

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

	std::random_device dev;
	std::default_random_engine rng;
	std::uniform_real_distribution<f32> uniform01 = std::uniform_real_distribution<f32>(0.0f, 1.0f);
};

Vec2 vectorInTangentSpaceBasis(Vec3 v, Vec3 tangentU, Vec3 tangentV, Vec3 normal);
Vec2 vectorInTangentSpaceBasis(Vec3 v, Vec3 tangentU, Vec3 tangentV);