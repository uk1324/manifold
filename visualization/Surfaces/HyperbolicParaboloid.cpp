#include "HyperbolicParaboloid.hpp"

Vec3 HyperbolicParaboloid::position(f32 u, f32 v) const {
	Vec3 m(0.0f);
	f32* out = m.data();
	out[0] = u;
	out[1] = pow(u, 2) - pow(v, 2);
	out[2] = v;
	return m;
}

Vec3 HyperbolicParaboloid::tangentU(f32 u, f32 v) const {
	Vec3 m(0.0f);
	f32* out = m.data();
	out[0] = 1;
	out[1] = 2 * u;
	out[2] = 0;
	return m;
}

Vec3 HyperbolicParaboloid::tangentV(f32 u, f32 v) const {
	Vec3 m(0.0f);
	f32* out = m.data();
	out[0] = 0;
	out[1] = -2 * v;
	out[2] = 1;
	return m;
}

Vec3 HyperbolicParaboloid::normal(f32 u, f32 v) const {
	Vec3 m(0.0f);
	f32* out = m.data();
	out[0] = 2 * u;
	out[1] = -1;
	out[2] = -2 * v;
	return m.normalized();
}

ChristoffelSymbols HyperbolicParaboloid::christoffelSymbols(f32 u, f32 v) const {
	f32 x0 = 4 / (4 * pow(u, 2) + 4 * pow(v, 2) + 1);
	f32 x1 = u * x0;
	f32 x2 = v * x0;
	Mat2 x = Mat2::identity, y = Mat2::identity;
	f32* out = x.data();
	out[0] = x1;
	out[1] = 0;
	out[2] = 0;
	out[3] = -x1;
	out = y.data();
	out[0] = -x2;
	out[1] = 0;
	out[2] = 0;
	out[3] = x2;
	return { .x = x, .y = y };
}

Mat2 HyperbolicParaboloid::firstFundamentalForm(f32 u, f32 v) const {
	Mat2 m(Vec2(0.0f), Vec2(0.0f));
	f32* out = m.data();
	f32 x0 = -4 * u * v;
	out[0] = 4 * pow(u, 2) + 1;
	out[1] = x0;
	out[2] = x0;
	out[3] = 4 * pow(v, 2) + 1;
	return m;
}

Mat2 HyperbolicParaboloid::secondFundamentalForm(f32 u, f32 v) const {
	Mat2 m(Vec2(0.0f), Vec2(0.0f));
	f32* out = m.data();
	f32 x0 = 2 / sqrt(4 * pow(u, 2) + 4 * pow(v, 2) + 1);
	out[0] = -x0;
	out[1] = 0;
	out[2] = 0;
	out[3] = x0;
	return m;
}

f32 HyperbolicParaboloid::curvature(f32 u, f32 v) const {
	f32 out;
	out = -4 / (16 * pow(u, 4) + 32 * pow(u, 2) * pow(v, 2) + 8 * pow(u, 2) + 16 * pow(v, 4) + 8 * pow(v, 2) + 1);
	return out;
}

PrincipalCurvatures HyperbolicParaboloid::principalCurvatures(f32 u, f32 v) const {
	// Using a numerical approximation, because the formulas are too long.
	return principalCurvatues(firstFundamentalForm(u, v), secondFundamentalForm(u, v));
}