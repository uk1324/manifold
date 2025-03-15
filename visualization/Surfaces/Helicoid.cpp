#include "Helicoid.hpp"

Vec3 Helicoid::position(f32 u, f32 v) const {
	return Vec3(
		u * cos(v),
		u * sin(v),
		v
	);
}

Vec3 Helicoid::tangentU(f32 u, f32 v) const {
	return Vec3(
		cos(v),
		sin(v),
		0
	);
}

Vec3 Helicoid::tangentV(f32 u, f32 v) const {
	return Vec3(
		-u * sin(v),
		u * cos(v),
		1
	);
}

Vec3 Helicoid::normal(f32 u, f32 v) const {
	return Vec3(
		sin(v),
		-cos(v),
		u
	).normalized();
}

ChristoffelSymbols Helicoid::christoffelSymbols(f32 u, f32 v) const {
	const auto a = u / (1 + u * u);
	return {
		.x = Mat2(
			Vec2(0.0f, 0.0f),
			Vec2(0.0f, -u)
		),
		.y = Mat2(
			Vec2(0, a),
			Vec2(a, 0)
		)
	};
}

f32 Helicoid::curvature(f32 u, f32 v) const {
	const auto a = (1.0f + u * u);
	return -1.0f / (a * a);
}

PrincipalCurvatures Helicoid::principalCurvatures(f32 u, f32 v) const {
	const auto k = 1 / (1.0f + u * u);
	return PrincipalCurvatures(
		k, Vec2(1.0f, 0.0f),
		-k, Vec2(0.0f, 1.0f)
	);
}
