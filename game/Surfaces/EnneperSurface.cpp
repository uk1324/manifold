#include "EnneperSurface.hpp"

Vec3 EnneperSurface::position(f32 u, f32 v) const {
	Vec3 m(0.0f);
	f32* out = m.data();
	f32 x0 = pow(v, 2);
	f32 x1 = pow(u, 2);
	out[0] = u * (x0 - 1.0 / 3.0 * x1 + 1);
	out[1] = v * (-1.0 / 3.0 * x0 + x1 + 1);
	out[2] = -x0 + x1;
	return m ;
}

Vec3 EnneperSurface::tangentU(f32 u, f32 v) const {
	Vec3 m(0.0f);
	f32* out = m.data();
	f32 x0 = 2 * u;
	out[0] = -pow(u, 2) + pow(v, 2) + 1;
	out[1] = v * x0;
	out[2] = x0;
	return m;
}

Vec3 EnneperSurface::tangentV(f32 u, f32 v) const {
	Vec3 m(0.0f);
	f32* out = m.data();
	f32 x0 = 2 * v;
	out[0] = u * x0;
	out[1] = pow(u, 2) - pow(v, 2) + 1;
	out[2] = -x0;
	return m;
}

Vec3 EnneperSurface::normal(f32 u, f32 v) const {
	Vec3 m(0.0f);
	f32* out = m.data();
	out[0] = -2 * u;
	out[1] = 2 * v;
	out[2] = -pow(u, 2) - pow(v, 2) + 1;
	return m.normalized();
}

ChristoffelSymbols EnneperSurface::christoffelSymbols(f32 u, f32 v) const {
	f32 x0 = 2 / (pow(u, 2) + pow(v, 2) + 1);
	f32 x1 = u * x0;
	f32 x2 = v * x0;
	Mat2 x = Mat2::identity, y = Mat2::identity;
	f32* out = x.data();
	out[0] = x1;
	out[1] = x2;
	out[2] = x2;
	out[3] = -x1;
	out = y.data();
	out[0] = -x2;
	out[1] = x1;
	out[2] = x1;
	out[3] = x2;
	return { .x = x, .y = y };
}
Mat2 EnneperSurface::firstFundamentalForm(f32 u, f32 v) const {
	Mat2 m(Vec2(0.0f), Vec2(0.0f));
	f32* out = m.data();
	f32 x0 = pow(u, 2);
	f32 x1 = 4 * x0;
	f32 x2 = pow(v, 2);
	f32 x3 = x1 * x2;
	out[0] = x1 + x3 + pow(-x0 + x2 + 1, 2);
	out[1] = 0;
	out[2] = 0;
	out[3] = 4 * x2 + x3 + pow(x0 - x2 + 1, 2);
	return m;
}

Mat2 EnneperSurface::secondFundamentalForm(f32 u, f32 v) const {
	Mat2 m(Vec2(0.0f), Vec2(0.0f));
	f32* out = m.data();
	f32 x0 = pow(u, 2);
	f32 x1 = pow(v, 2);
	f32 x2 = x0 + x1;
	f32 x3 = x2 + 1;
	f32 x4 = 2 / sqrt(4 * x0 + 4 * x1 + pow(x2 - 1, 2));
	out[0] = x3 * x4;
	out[1] = 0;
	out[2] = 0;
	out[3] = -x3 * x4;
	return m;
}

PrincipalCurvatures EnneperSurface::principalCurvatures(f32 u, f32 v) const {
	const auto k = 2 / ((pow(u, 2) + pow(v, 2) + 1) * sqrt(pow(u, 4) + 2 * pow(u, 2) * pow(v, 2) + 2 * pow(u, 2) + pow(v, 4) + 2 * pow(v, 2) + 1));
	return PrincipalCurvatures(
		-k,
		Vec2(1.0f, 0.0f),
		k,
		Vec2(0.0f, 1.0f)
	);
}

f32 EnneperSurface::curvature(f32 u, f32 v) const {
	f32 out;
	out = -4 / (pow(pow(u, 2) + pow(v, 2) + 1, 2) * (pow(u, 4) + 2 * pow(u, 2) * pow(v, 2) + 2 * pow(u, 2) + pow(v, 4) + 2 * pow(v, 2) + 1));
	return out;
}

