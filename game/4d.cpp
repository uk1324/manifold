#include "4d.hpp"
#include <engine/Math/Quat.hpp>

Vec4 projectVectorToSphereTangentSpace(Vec4 pointOnSphere, Vec4 anyVector) {
	const auto& normalToTangentSpace = pointOnSphere.normalized();
	const auto d = dot(anyVector, normalToTangentSpace);
	anyVector -= normalToTangentSpace * d;
	return anyVector;
}

Vec4 moveForwardOnSphere(Vec4 initialPosition, Vec4 direction) {
	const auto p = Quat(initialPosition.x, initialPosition.y, initialPosition.z, initialPosition.w).normalized();
	auto v = Quat(direction.x, direction.y, direction.z, direction.w);
	// Move to identity
	v *= p.inverseIfNormalized();

	Quat movement = quatExp(Vec3(v.x, v.y, v.z));
	Quat r = (p * movement).normalized();
	return Vec4(r.x, r.y, r.z, r.w);
}
