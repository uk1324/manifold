#include "Math.hpp"
#include <engine/Math/GramSchmidt.hpp>
#include <algorithm>

f32 det(
	f32 m00, f32 m10,
	f32 m01, f32 m12) {
	return m00 * m12 - m10 * m01;
}

f32 det(
	f32 m00, f32 m10, f32 m20,
	f32 m01, f32 m11, f32 m21,
	f32 m02, f32 m12, f32 m22) {
	return
		+m00 * det(m11, m21, m12, m22)
		- m10 * det(m01, m21, m02, m22)
		+ m20 * det(m01, m11, m02, m12);
}
f32 det(Vec3 v0, Vec3 v1, Vec3 v2) {
	return det(
		v0.x, v0.y, v0.z,
		v1.x, v1.y, v1.z,
		v2.x, v2.y, v2.z
	);
}

// Vector perpendicular the plane spanned by v0, v1, v2
Vec4 crossProduct(Vec4 v0, Vec4 v1, Vec4 v2) {
	/*
	| e0   e1   e2   e3   |
	| v0.x v0.y v0.z v0.w | = cross(v0, v1, v2)
	| v1.x v1.y v1.z v1.w |
	| v2.x v2.y v2.z v2.w |
	*/
	return Vec4(
		det(
			v0.y, v0.z, v0.w,
			v1.y, v1.z, v1.w,
			v2.y, v2.z, v2.w
		),
		-det(
			v0.x, v0.z, v0.w,
			v1.x, v1.z, v1.w,
			v2.x, v2.z, v2.w
		),
		det(
			v0.x, v0.y, v0.w,
			v1.x, v1.y, v1.w,
			v2.x, v2.y, v2.w
		),
		-det(
			v0.x, v0.y, v0.z,
			v1.x, v1.y, v1.z,
			v2.x, v2.y, v2.z
		)
	);
}

Vec3 coordinatesInOrthonormal3Basis(const Vec4 orthonormalBasis[3], Vec4 v) {
	return Vec3(
		dot(v, orthonormalBasis[0]),
		dot(v, orthonormalBasis[1]),
		dot(v, orthonormalBasis[2])
	);
}

Vec4 linearCombination(const Vec4 vs[3], Vec3 v) {
	return vs[0] * v.x + vs[1] * v.y + vs[2] * v.z;
}

std::array<Vec4, 3> orthonormalBasisFor3Space(Vec4 normal) {
	Vec4 candidates[]{
		Vec4(1.0f, 0.0f, 0.0f, 0.0f),
		Vec4(0.0f, 1.0f, 0.0f, 0.0f),
		Vec4(0.0f, 0.0f, 1.0f, 0.0f),
		Vec4(0.0f, 0.0f, 0.0f, 1.0f)
	};
	//f32 candidatesLengths[4]{};
	for (i32 i = 0; i < 4; i++) {
		auto& candidate = candidates[i];
		candidate -= dot(candidate, normal) * normal;
		//candidatesLengths[i] = candidate.length();
	}
	// @Performance: Calculate lengths once and sort an array of indices.
	std::ranges::sort(candidates, [](Vec4 a, Vec4 b) -> bool {
		return a.length() < b.length();
	});
	gramSchmidtOrthonormalize(View<Vec4>(candidates, 3));
	return std::array<Vec4, 3>{
		candidates[0],
			candidates[1],
			candidates[2]
	};
}

std::array<Vec4, 2> basisForOrthogonalComplement(Vec4 v0, Vec4 v1) {
	// alternative
	// https://www.geometrictools.com/Documentation/OrthonormalSets.pdf
	// Orthonormalize plane basis.
	v0 = v0.normalized();
	v1 -= dot(v1, v0) * v0;
	v1 = v1.normalized();

	Vec4 candidates[]{
		Vec4(1.0f, 0.0f, 0.0f, 0.0f),
		Vec4(0.0f, 1.0f, 0.0f, 0.0f),
		Vec4(0.0f, 0.0f, 1.0f, 0.0f),
		Vec4(0.0f, 0.0f, 0.0f, 1.0f)
	};
	for (i32 i = 0; i < 4; i++) {
		auto& candidate = candidates[i];
		candidate -= dot(candidate, v0) * v0;
		candidate -= dot(candidate, v1) * v1;
	}
	// @Performance: Calculate lengths once and sort an array of indices.
	std::ranges::sort(candidates, [](Vec4 a, Vec4 b) -> bool {
		return a.length() > b.length();
	});
	gramSchmidtOrthonormalize(View<Vec4>(candidates, 2));
	return std::array<Vec4, 2>{
		candidates[0],
		candidates[1],
	};
}

// a and b are points on the unit sphere
Quat unitSphereRotateAToB(Vec3 a, Vec3 b) {
	auto rotationAxis = cross(a, b).normalized();
	auto rotationAngle = acos(std::clamp(dot(a,
		b.normalized()), -1.0f, 1.0f));

	if (std::abs(rotationAngle) < 0.01f) {
		rotationAngle = 0.0f;
		rotationAxis = Vec3(1.0f, 0.0f, 0.0f);
	}
	return Quat(rotationAngle, rotationAxis);
}

f32 randomF32m1To1() {
	return 2.0f * (f32(rand()) / f32(RAND_MAX) - 0.5f);
}

Vec4 randomVec4m1To1() {
	return Vec4(
		randomF32m1To1(),
		randomF32m1To1(),
		randomF32m1To1(),
		randomF32m1To1()
	);
}
