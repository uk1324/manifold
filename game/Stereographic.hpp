#pragma once

#include <engine/Math/Vec4.hpp>
#include <engine/Math/Sphere.hpp>
#include <engine/Math/Plane.hpp>
#include <View.hpp>

bool isPointAtInfinity(Vec3 v);

// Projects from a point on dot(p, p) = 1 to 3D.
Vec3 stereographicProjection(Vec4 p);
Vec3 stereographicProjectionJacobian(Vec4 atPoint, Vec4 onVector);

Vec4 inverseStereographicProjection(Vec3 p);
Vec4 inverseStereographicProjectionJacobian(Vec3 at, Vec3 of);

Vec3 antipodalPoint(Vec3 p);

struct CircularSegment {
	CircularSegment(Vec3 start, Vec3 initialVelocity, Vec3 center, f32 angle);

	Vec3 start;
	Vec3 initialVelocity;
	Vec3 center;
	f32 angle;

	Vec3 sample(f32 angle) const;
};

struct LineSegment {
	LineSegment(Vec3 e0, Vec3 e1);
	Vec3 e[2];
};

struct StereographicSegment {
private:
	StereographicSegment(Vec3 start, Vec3 initialVelocity, Vec3 center, f32 angle);
	StereographicSegment(Vec3 e0, Vec3 e1);
public:
	static StereographicSegment fromCircular(Vec3 start, Vec3 initialVelocity, Vec3 center, f32 angle);
	static StereographicSegment fromLine(Vec3 e0, Vec3 e1);

	static StereographicSegment fromEndpoints(Vec4 e0, Vec4 e1);

	enum class Type {
		LINE, CIRCULAR
	};
	Type type;
	union {
		CircularSegment circular;
		LineSegment line;
	};
};

struct StereographicPlane {
	static StereographicPlane fromVertices(Vec4 v0, Vec4 v1, Vec4 v2);

	enum class Type {
		SPHERE, PLANE
	};
	Type type;
	union {
		Plane plane;
		Sphere sphere;
	};
};

std::optional<f32> rayStereographicPlaneIntersection(Vec3 rayOrigin, Vec3 rayDirection, const StereographicPlane& 
 plane);
std::optional<f32> rayStereographicPlaneIntersection(const Ray3& ray, const StereographicPlane& plane);

std::optional<f32> rayStereographicPolygonIntersection(Vec3 rayOrigin, Vec3 rayDirection, const StereographicPlane&
	plane, View<const Vec4> edgeNormals);
std::optional<f32> rayStereographicPolygonIntersection(const Ray3& ray, const StereographicPlane&
	plane, View<const Vec4> edgeNormals);