#pragma once

#include <engine/Math/Vec3.hpp>
#include <engine/Math/Vec2.hpp>
#include <game/Tri3d.hpp>
#include <vector>

Vec2 vectorInTangentSpaceBasis(Vec3 v, Vec3 tangentU, Vec3 tangentV, Vec3 normal);

struct MeshIntersection {
	RayTriIntersection i;
	i32 triangleIndex;
	Vec2 uv;
	Vec3 position;
};
void sortIntersectionsByDistanceToCamera(std::vector<MeshIntersection>& intersections);

struct PointGrabbed {
	Vec3 position;
};

bool checkIfPointGotGrabbed(
	Vec3 pointPosition,
	const std::vector<MeshIntersection>& intersectionsSortedByDistance);
void updateGrabbedPoint(
	Vec2& pointUv,
	Vec3 pointPos,
	std::vector<MeshIntersection>& intersections);