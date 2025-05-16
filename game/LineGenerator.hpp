#pragma once

#include <engine/Math/Vec3.hpp>
#include <engine/Math/Vec2.hpp>
#include <game/Stereographic.hpp>
#include <vector>

struct LineGenerator {
	void addLine(const std::vector<Vec3>& curvePoints);
	void addPlaneCurve(const std::vector<Vec3>& curvePoints, Vec3 planeNormal);
	void addFlatCurve(const std::vector<Vec3>& curvePoints, Vec3 cameraForward);
	void addCircularArc(Vec3 a, Vec3 b, Vec3 circleCenter, f32 tubeRadius);
	void addCircularArc(Vec3 aRelativeToCenter, Vec3 velocityOutOfA, Vec3 circleCenter, f32 arclength, f32 tubeRadius);

	void addStereographicArc(const StereographicSegment& segment, f32 radius);

	void reset();
	i32 vertexCount() const;
	std::vector<Vec3> positions;
	std::vector<Vec3> normals;
	std::vector<Vec3> uvs; // u is the length along the curve, v i the angle / 2pi around the curve.
	std::vector<i32> indices;
};