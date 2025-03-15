#include "Pseudosphere.hpp"

Vec3 Pseudosphere::position(f32 u, f32 v) const {
	Vec3 m(0.0f);
	f32* out = m.data();
	f32 x0 = r * 1.0 / ((1.0 / 2.0) * exp(u) + (1.0 / 2.0) * exp(-u));
	out[0] = x0 * cos(v);
	out[1] = x0 * sin(v);
	out[2] = r * u - r * tanh(u);
	return m;
}

Vec3 Pseudosphere::tangentU(f32 u, f32 v) const {
	Vec3 m(0.0f);
	f32* out = m.data();
	f32 x0 = tanh(u);
	f32 x1 = r * x0 * 1.0 / ((1.0 / 2.0) * exp(u) + (1.0 / 2.0) * exp(-u));
	out[0] = -x1 * cos(v);
	out[1] = -x1 * sin(v);
	out[2] = r * pow(x0, 2);
	return m;
}

Vec3 Pseudosphere::tangentV(f32 u, f32 v) const {
	Vec3 m(0.0f);
	f32* out = m.data();
	f32 x0 = r * 1.0 / ((1.0 / 2.0) * exp(u) + (1.0 / 2.0) * exp(-u));
	out[0] = -x0 * sin(v);
	out[1] = x0 * cos(v);
	out[2] = 0;
	return m;
}

Vec3 Pseudosphere::normal(f32 u, f32 v) const {
	Vec3 m(0.0f);
	f32* out = m.data();
	f32 x0 = tanh(u);
	out[0] = -x0 * cos(v);
	out[1] = -x0 * sin(v);
	out[2] = -1 / cosh(u);
	return m.normalized();
}

ChristoffelSymbols Pseudosphere::christoffelSymbols(f32 u, f32 v) const {
	f32 x0 = tanh(u);
	f32 x1 = -x0;
	Mat2 x = Mat2::identity, y = Mat2::identity;
	f32* out = x.data();
	out[0] = -x0 + 1.0 / x0;
	out[1] = 0;
	out[2] = 0;
	out[3] = 2 / sinh(2 * u);
	out = y.data();
	out[0] = 0;
	out[1] = x1;
	out[2] = x1;
	out[3] = 0;
	return { .x = x, .y = y };
}

f32 Pseudosphere::curvature(f32 u, f32 v) const {
	return -1.0f / (r * r);
}

PrincipalCurvatures Pseudosphere::principalCurvatures(f32 u, f32 v) const {
	return PrincipalCurvatures(
		-(1.0f / r) * abs(cosh(u)),
		Vec2(1.0f, 0.0f),
		(1.0f / r) * abs(sinh(u)),
		Vec2(0.0f, 1.0f)
	);
}

