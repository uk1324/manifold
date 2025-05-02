#pragma once

#include <engine/Math/Vec4.hpp>


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