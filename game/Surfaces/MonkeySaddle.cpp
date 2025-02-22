#include "MonkeySaddle.hpp"

Vec3 MonkeySaddle::position(f32 u, f32 v) const {
	Vec3 m(0.0f);
	f32* out = m.data();
	out[0] = u;
	out[1] = pow(u, 3) - 3 * u * pow(v, 2);
	out[2] = v;
	return m;
}

Vec3 MonkeySaddle::tangentU(f32 u, f32 v) const {
	Vec3 m(0.0f);
	f32* out = m.data();
	out[0] = 1;
	out[1] = 3 * pow(u, 2) - 3 * pow(v, 2);
	out[2] = 0;
	return m;
}

Vec3 MonkeySaddle::tangentV(f32 u, f32 v) const {
	Vec3 m(0.0f);
	f32* out = m.data();
	out[0] = 0;
	out[1] = -6 * u * v;
	out[2] = 1;
	return m;
}

Vec3 MonkeySaddle::normal(f32 u, f32 v) const {
	Vec3 m(0.0f);
	f32* out = m.data();
	out[0] = 3 * pow(u, 2) - 3 * pow(v, 2);
	out[1] = -1;
	out[2] = -6 * u * v;
	return m.normalized();
}

ChristoffelSymbols MonkeySaddle::christoffelSymbols(f32 u, f32 v) const {
	f32 x0 = pow(u, 2);
	f32 x1 = pow(v, 2);
	f32 x2 = x0 - x1;
	f32 x3 = 1.0 / (9 * pow(u, 4) + 9 * pow(v, 4) + 18 * x0 * x1 + 1);
	f32 x4 = 18 * x3;
	f32 x5 = u * x4;
	f32 x6 = -x2;
	f32 x7 = v * x4 * x6;
	f32 x8 = 36 * x3;
	f32 x9 = v * x0 * x8;
	f32 x10 = u * x1 * x8;
	Mat2 x = Mat2::identity, y = Mat2::identity;
	f32* out = x.data();
	out[0] = x2 * x5;
	out[1] = x7;
	out[2] = x7;
	out[3] = x5 * x6;
	out = y.data();
	out[0] = -x9;
	out[1] = x10;
	out[2] = x10;
	out[3] = x9;
	return { .x = x, .y = y };
}
Mat2 MonkeySaddle::firstFundamentalForm(f32 u, f32 v) const {
	Mat2 m(Vec2(0.0f), Vec2(0.0f));
	f32* out = m.data();
	f32 x0 = pow(u, 2);
	f32 x1 = pow(v, 2);
	f32 x2 = x0 - x1;
	f32 x3 = -18 * u * v * x2;
	out[0] = 9 * pow(x2, 2) + 1;
	out[1] = x3;
	out[2] = x3;
	out[3] = 36 * x0 * x1 + 1;
	return m;
}

Mat2 MonkeySaddle::secondFundamentalForm(f32 u, f32 v) const {
	Mat2 m(Vec2(0.0f), Vec2(0.0f));
	f32* out = m.data();
	f32 x0 = pow(u, 2);
	f32 x1 = pow(v, 2);
	f32 x2 = 6 / sqrt(36 * x0 * x1 + 9 * pow(x0 - x1, 2) + 1);
	f32 x3 = u * x2;
	f32 x4 = v * x2;
	out[0] = -x3;
	out[1] = x4;
	out[2] = x4;
	out[3] = x3;
	return m;
}

PrincipalCurvatures MonkeySaddle::principalCurvatures(f32 u, f32 v) const {
	return principalCurvatues(firstFundamentalForm(u, v), secondFundamentalForm(u, v));
}

f32 MonkeySaddle::curvature(f32 u, f32 v) const {
	f32 out;
	out = 36 * (-pow(u, 2) - pow(v, 2)) / (81 * pow(u, 8) + 324 * pow(u, 6) * pow(v, 2) + 486 * pow(u, 4) * pow(v, 4) + 18 * pow(u, 4) + 324 * pow(u, 2) * pow(v, 6) + 36 * pow(u, 2) * pow(v, 2) + 81 * pow(v, 8) + 18 * pow(v, 4) + 1);
	return out;
}

