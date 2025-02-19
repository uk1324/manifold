#include "Utils.hpp"
#include <engine/Math/Mat2.hpp>
#include <engine/Input/Input.hpp>

Vec2 vectorInTangentSpaceBasis(Vec3 v, Vec3 tangentU, Vec3 tangentV, Vec3 normal) {
	// Untimatelly this requires solving the system of equations a0 tV + a1 tU = v so I don't think there is any better way of doing this.
	const auto v0 = tangentU.normalized();
	const auto v1 = cross(tangentU, normal).normalized();
	auto toOrthonormalBasis = [&](Vec3 v) -> Vec2 {
		return Vec2(dot(v, v0), dot(v, v1));
	};
	// Instead of doing this could for example use the Moore Penrose pseudoinverse or some other method for system with no solution. One advantage of this might be that it always gives some value instead of doing division by zero in points where the surface is not regular, but it is probably also more expensive, because it requires an inverse of a 3x3 matrix.
	const auto tU = toOrthonormalBasis(tangentU);
	const auto tV = toOrthonormalBasis(tangentV);
	const auto i = toOrthonormalBasis(v);
	const auto inUvCoordinates = Mat2(tU, tV).inversed() * i;
	return inUvCoordinates;
}

void sortIntersectionsByDistanceToCamera(std::vector<MeshIntersection>& intersections) {
	std::ranges::sort(
		intersections,
		[](const MeshIntersection& a, const MeshIntersection& b) {
			return a.i.t < b.i.t;
		}
	);
}

bool checkIfPointGotGrabbed(
	Vec3 pointPosition,
	const std::vector<MeshIntersection>& intersectionsSortedByDistance) {

	const auto grabDistance = 0.06f;
	// Counting all intersections so the user can grab things on the other side of the transparent surface.
	for (const auto& intersection : intersectionsSortedByDistance) {
		if (intersection.position.distanceTo(pointPosition) < grabDistance) {
			return true;
		}
	}
	return false;
}

void updateGrabbedPoint(
	Vec2& pointUv, 
	Vec3 pointPos, 
	std::vector<MeshIntersection>& intersections){
	// Sorting by the distance to the current position so that if the user grabs the thing on the other side it stays on the other side.
	std::ranges::sort(
		intersections,
		[&pointPos](const MeshIntersection& a, const MeshIntersection& b) {
			// Could precompute.
			return a.i.position.distanceTo(pointPos) < b.i.position.distanceTo(pointPos);
		}
	);
	if (intersections.size() >= 1) {
		pointUv = intersections[0].uv;
	}
}
