#pragma once

#include <engine/Math/Vec4.hpp>
#include <array>
#include <engine/Math/Quat.hpp>

f32 det(
	f32 m00, f32 m10,
	f32 m01, f32 m12);

f32 det(
	f32 m00, f32 m10, f32 m20,
	f32 m01, f32 m11, f32 m21,
	f32 m02, f32 m12, f32 m22);
f32 det(Vec3 v0, Vec3 v1, Vec3 v2);

// Vector perpendicular the plane spanned by v0, v1, v2
Vec4 crossProduct(Vec4 v0, Vec4 v1, Vec4 v2);

f32 planeRayIntersection(Vec4 planeNormal, f32 planeD, Vec4 rayOrigin, Vec4 rayDirection);

Vec3 coordinatesInOrthonormal3Basis(const Vec4 orthonormalBasis[3], Vec4 v);

Vec4 linearCombination(const Vec4 vs[3], Vec3 v);

std::array<Vec4, 3> orthonormalBasisFor3Space(Vec4 normal);
std::array<Vec4, 2> basisForOrthogonalComplement(Vec4 v0, Vec4 v1);
// a and b are points on the unit sphere
Quat unitSphereRotateAToB(Vec3 a, Vec3 b);