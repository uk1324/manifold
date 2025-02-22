#include "Catenoid.hpp"

Vec3 Catenoid::position(f32 u, f32 v) const {
	Vec3 m(0.0f);
	f32* out = m.data();
	f32 x0 = c * cosh(v / c);
	out[0] = x0 * cos(u);
	out[1] = x0 * sin(u);
	out[2] = v;
	return m;
}

Vec3 Catenoid::tangentU(f32 u, f32 v) const {
	Vec3 m(0.0f);
	f32* out = m.data();
	f32 x0 = c * cosh(v / c);
	out[0] = -x0 * sin(u);
	out[1] = x0 * cos(u);
	out[2] = 0;
	return m;
}

Vec3 Catenoid::tangentV(f32 u, f32 v) const {
	Vec3 m(0.0f);
	f32* out = m.data();
	f32 x0 = sinh(v / c);
	out[0] = x0 * cos(u);
	out[1] = x0 * sin(u);
	out[2] = 1;
	return m;
}

Vec3 Catenoid::normal(f32 u, f32 v) const {
	Vec3 m(0.0f);
	f32* out = m.data();
	out[0] = cos(u);
	out[1] = sin(u);
	out[2] = -sinh(v / c);
	return m.normalized();
}

ChristoffelSymbols Catenoid::christoffelSymbols(f32 u, f32 v) const {
	f32 x0 = 1.0 / c;
	f32 x1 = tanh(v * x0);
	f32 x2 = x0 * x1;
	Mat2 x = Mat2::identity, y = Mat2::identity;
	f32* out = x.data();
	out[0] = 0;
	out[1] = x2;
	out[2] = x2;
	out[3] = 0;
	out = y.data();
	out[0] = -c * x1;
	out[1] = 0;
	out[2] = 0;
	out[3] = x2;
	return { .x = x, .y = y };
}
Mat2 Catenoid::firstFundamentalForm(f32 u, f32 v) const {
	Mat2 m(Vec2(0.0f), Vec2(0.0f));
	f32* out = m.data();
	f32 x0 = pow(cosh(v / c), 2);
	out[0] = pow(c, 2) * x0;
	out[1] = 0;
	out[2] = 0;
	out[3] = x0;
	return m;
}

Mat2 Catenoid::secondFundamentalForm(f32 u, f32 v) const {
	Mat2 m(Vec2(0.0f), Vec2(0.0f));
	f32* out = m.data();
	f32 x0 = 1.0 / c;
	f32 x1 = cosh(v * x0);
	f32 x2 = x1 / sqrt(pow(x1, 2));
	out[0] = -c * x2;
	out[1] = 0;
	out[2] = 0;
	out[3] = x0 * x2;
	return m;
}

PrincipalCurvatures Catenoid::principalCurvatures(f32 u, f32 v) const {
	const auto k = 1.0f / (c * sqrt(pow(cosh(v / c), 2.0f)) * cosh(v / c));
	return PrincipalCurvatures(-k, Vec2(1.0f, 0.0f), k, Vec2(0.0f, 1.0f));
}

f32 Catenoid::curvature(f32 u, f32 v) const {
	f32 out;
	out = -1 / (pow(c, 2) * pow(cosh(v / c), 4));
	return out;
}

