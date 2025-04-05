#include "Stereographic.hpp"

Vec2 toStereographic(Vec3 p) {
	using CalculationType = f64;
	const auto x = CalculationType(p.x);
	const auto y = CalculationType(p.y);
	const auto z = CalculationType(p.z);
	const auto d = CalculationType(1) - z;
	const auto X = x / d;
	const auto Y = y / d;
	return Vec2(f32(X), f32(Y));

	/*const auto d = 1.0f - p.z;
	return Vec2(p.x / d, p.y / d);*/
}