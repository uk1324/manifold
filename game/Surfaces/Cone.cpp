#include "Cone.hpp"

Vec3 Cone::position(f32 u, f32 v) const {
	return Vec3(
		a * u * cos(v),
		b * u * sin(v),
		u
	);
}

Vec3 Cone::tangentU(f32 u, f32 v) const {
	return Vec3(
		a * cos(v),
		a * sin(v),
		1
	);
}

Vec3 Cone::tangentV(f32 u, f32 v) const {
	return Vec3(
		-a * u * sin(v),
		a * u * cos(v),
		0
	);
}

Vec3 Cone::normal(f32 u, f32 v) const {
	return Vec3(
		-b * u * cos(v),
		-a * u * sin(v),
		a * b * u
	).normalized();
}

ChristoffelSymbols Cone::christoffelSymbols(f32 u, f32 v) const {
	const auto a = 1.0f / u;
	return {
		.x = Mat2(
			Vec2(0.0f, 0.0f),
			Vec2(0.0f, -a * a * u / (a * a + 1.0f))
		),
		.y = Mat2(
			Vec2(0.0f, u),
			Vec2(u, 0.0f)
		)
	};
}

f32 Cone::curvature(f32 u, f32 v) const {
	return 0.0f;
}

PrincipalCurvatures Cone::principalCurvatures(f32 u, f32 v) const {
	return PrincipalCurvatures(
		0.0f, 
		Vec2(1.0f, 0.0f),
		1.0f / (a * abs(u) * sqrt(1.0f + a * a)), 
		Vec2(0.0f, 1.0f)
	);
}
