#include "4d.hpp"
#include <engine/Math/Quat.hpp>
#include <game/Math.hpp>

Vec4 quatMul(Vec4 a, Vec4 b) {
	const auto r = Quat(a.x, a.y, a.z, a.w) * Quat(b.x, b.y, b.z, b.w);
	return Vec4(r.x, r.y, r.z, r.w);
}

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

Vec4 movementForwardOnSphereQuick(Vec4 initialPosition, Vec4 direction) {
	const auto p = Quat(initialPosition.x, initialPosition.y, initialPosition.z, initialPosition.w).normalized();
	auto v = Quat(direction.x, direction.y, direction.z, direction.w);
	// Move to identity
	v *= p.inverseIfNormalized();
	Quat movement = quatExp(Vec3(v.x, v.y, v.z));
	return Vec4(movement.x, movement.y, movement.z, movement.w);
}

Mat4 movementForwardOnSphere(Vec4 initialPosition, Vec4 direction) {
	const auto rotationPlaneBasis0 = initialPosition.normalized();
	auto rotationPlaneBasis1 = direction;
	rotationPlaneBasis1 -= dot(rotationPlaneBasis0, rotationPlaneBasis1) * rotationPlaneBasis0;
	rotationPlaneBasis1 = rotationPlaneBasis1.normalized();
	const auto speed = direction.length();
	if (speed == 0.0f) {
		return Mat4::identity;
	}

	const auto orthogonalPlaneBasis = basisForOrthogonalComplement(rotationPlaneBasis0, rotationPlaneBasis1);

	auto vectorToBasis = [&](Vec4 v) -> Vec4 {
		return Vec4(
			dot(v, rotationPlaneBasis0),
			dot(v, rotationPlaneBasis1),
			dot(v, orthogonalPlaneBasis[0]),
			dot(v, orthogonalPlaneBasis[1])
		);
	};
	const auto v0 = vectorToBasis(Vec4(1.0f, 0.0f, 0.0f, 0.0f));
	const auto v1 = vectorToBasis(Vec4(0.0f, 1.0f, 0.0f, 0.0f));
	const auto v2 = vectorToBasis(Vec4(0.0f, 0.0f, 1.0f, 0.0f));
	const auto v3 = vectorToBasis(Vec4(0.0f, 0.0f, 0.0f, 1.0f));
	auto vectorFromBasis = [&](Vec4 v) -> Vec4 {
		return 
			v.x * rotationPlaneBasis0 +
			v.y * rotationPlaneBasis1 +
			v.z * orthogonalPlaneBasis[0] +
			v.w * orthogonalPlaneBasis[1];
	};
	auto transform = [&](Vec4 v) -> Vec4 {
		return Vec4(
			cos(speed) * v.x - sin(speed) * v.y,
			sin(speed) * v.x + cos(speed) * v.y,
			v.z,
			v.w
		);
	};
	return Mat4(
		vectorFromBasis(transform(v0)),
		vectorFromBasis(transform(v1)),
		vectorFromBasis(transform(v2)),
		vectorFromBasis(transform(v3))
	);
}

Vec4 normalizedDirectionFromAToB(Vec4 a, Vec4 b) {
	/*const auto p0 = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
	const auto p1 = inverseStereographicProjection(hit);*/
	Vec4 direction = b - dot(b, a) * a;
	direction = direction.normalized();
	return direction;
}

// https://www.physicsforums.com/threads/calculating-volume-of-a-parallelepiped-in-r-n.394684/

/*
Distance from a point to a line in 3D can be calculated by creating a parallelogram.
We have a line as a point and a vector. Then The sides will be the vector coming out of the origin of the line and the other side will be the vector connecting the origin to the other point. Then using the cross product you can calculate area of this parallelogram and the by dividing the base length get the height that is the distance between the line and the point.

This procedure can be generalized to higher dimensions. We can just consider a lower dimension subspace in which these shapes lie and then calculate things there (because projection in the orthogonal direction doesn't change lengths). 

This can also be calculated without explicitly changing basis. Formulas for volumes of k-parallelotopes in R^n are given by the Gram matrix determinant.
https://en.wikipedia.org/wiki/Gram_matrix#Gram_determinant
https://mathweb.ucsd.edu/~jeggers/math31ch/k_volume.pdf
https://math.stackexchange.com/questions/1419275/volume-of-a-n-d-parallelepiped-with-sides-given-by-the-row-vectors-of-a-matrix
https://math.stackexchange.com/questions/225304/volume-of-3d-triangular-based-pyramid-embedded-in-4-dimensions

The formula can be written with dot products or as det(A A^T).
*/

f32 parallelogramArea(Vec4 v0, Vec4 v1) {
	return sqrt(det(
		dot(v0, v0), dot(v0, v1),
		dot(v1, v0), dot(v1, v1)
	));
}

f32 parallelepipedArea(Vec4 v0, Vec4 v1, Vec4 v2) {
	return sqrt(det(
		dot(v0, v0), dot(v0, v1), dot(v0, v2),
		dot(v1, v0), dot(v1, v1), dot(v1, v2),
		dot(v2, v0), dot(v2, v1), dot(v2, v2)
	));
}

f32 distanceFromPlaneToPoint(Vec4 planePoint, Vec4 planeSpanning0, Vec4 planeSpanning1, Vec4 point) {
	return 
		parallelepipedArea(point - planePoint, planeSpanning0, planeSpanning1) / 
		parallelogramArea(planeSpanning0, planeSpanning1);
}
