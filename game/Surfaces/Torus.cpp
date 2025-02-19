#include "Torus.hpp"

// http://www.rdrop.com/~half/math/torus/geodesics.xhtml

// https://trecs.se/torus.php
Vec3 torusPosition(f32 u, f32 v, f32 r, f32 R) {
	return Vec3(
		(R + r * cos(v)) * cos(u),
		(R + r * cos(v)) * sin(u),
		r * sin(v)
	);
}

Vec3 torusTangentU(f32 u, f32 v, f32 r, f32 R) {
	return Vec3(
		-(R + r * cos(v)) * sin(u),
		(R + r * cos(v)) * cos(u),
		0.0f
	);
}

Vec3 torusTangentV(f32 u, f32 v, f32 r, f32 R) {
	return Vec3(
		-r * sin(v) * cos(u),
		-r * sin(v) * sin(u),
		r * cos(v)
	);
}

Vec3 torusNormal(f32 u, f32 v, f32 r, f32 R) {
	return Vec3(
		cos(u) * cos(v),
		sin(u) * cos(v),
		sin(v)
	).normalized();
}

ChristoffelSymbols torusChristoffelSymbols(f32 u, f32 v, f32 r, f32 R) {
	const auto a = -r * sin(v) / (R + r * cos(v));
	return {
		.x = Mat2(Vec2(0, a), Vec2(a, 0)),
		.y = Mat2(Vec2((R + r * cos(v)) * sin(v) / r, 0.0), Vec2(0.0f, 0.0f)),
	};
}

Vec3 Torus::position(f32 u, f32 v) const {
	return torusPosition(u, v, r, R);
}

Vec3 Torus::tangentU(f32 u, f32 v) const {
	return torusTangentU(u, v, r, R);
}

Vec3 Torus::tangentV(f32 u, f32 v) const {
	return torusTangentV(u, v, r, R);
}

Vec3 Torus::normal(f32 u, f32 v) const {
	return torusNormal(u, v, r, R);
}

ChristoffelSymbols Torus::christoffelSymbols(f32 u, f32 v) const {
	return torusChristoffelSymbols(u, v, r, R);
}

f32 Torus::curvature(f32 u, f32 v) const {
	return cos(v) / (r * (R + r * cos(v)));
}

PrincipalCurvatures Torus::principalCurvatures(f32 u, f32 v) const {
	return PrincipalCurvatures(
		-cos(v) / (R + r * cos(v)),
		Vec2(1.0f, 0.0f),
		-1.0f / r,
		Vec2(0.0f, 1.0f)
	);
}
