#include "MobiusStrip.hpp"

// https://trecs.se/M%C3%B6biusStrip.php

Vec3 MobiusStrip::position(f32 u, f32 v) const {
	Vec3 m(0.0f);
	f32* out = m.data();
	f32 x0 = (1.0 / 2.0) * u;
	f32 x1 = (1.0 / 2.0) * v;
	f32 x2 = x1 * cos(x0) + 1;
	out[0] = x2 * cos(u);
	out[1] = x2 * sin(u);
	out[2] = x1 * sin(x0);
	return m;
}

Vec3 MobiusStrip::tangentU(f32 u, f32 v) const {
	Vec3 m(0.0f);
	f32* out = m.data();
	f32 x0 = cos(u);
	f32 x1 = (1.0 / 2.0) * u;
	f32 x2 = (1.0 / 4.0) * v * sin(x1);
	f32 x3 = sin(u);
	f32 x4 = v * cos(x1);
	f32 x5 = x4 + 2;
	out[0] = -x0 * x2 - 1.0 / 2.0 * x3 * x5;
	out[1] = (1.0 / 2.0) * x0 * x5 - x2 * x3;
	out[2] = (1.0 / 4.0) * x4;
	return m;
}

Vec3 MobiusStrip::tangentV(f32 u, f32 v) const {
	Vec3 m(0.0f);
	f32* out = m.data();
	f32 x0 = (1.0 / 2.0) * u;
	f32 x1 = (1.0 / 2.0) * cos(x0);
	out[0] = x1 * cos(u);
	out[1] = x1 * sin(u);
	out[2] = (1.0 / 2.0) * sin(x0);
	return m;
}

Vec3 MobiusStrip::normal(f32 u, f32 v) const {
	Vec3 m(0.0f);
	f32* out = m.data();
	f32 x0 = (1.0 / 8.0) * v;
	f32 x1 = x0 * sin(u);
	f32 x2 = (1.0 / 2.0) * u;
	f32 x3 = (3.0 / 2.0) * u;
	f32 x4 = cos(u);
	f32 x5 = cos(x2);
	f32 x6 = (1.0 / 4.0) * x5;
	out[0] = x1 * x4 - x1 - 1.0 / 4.0 * sin(x2) + (1.0 / 4.0) * sin(x3);
	out[1] = -3.0 / 16.0 * v * pow(x4, 2) + (1.0 / 16.0) * v * x4 + x0 * x4 * pow(x5, 2) + x0 + x6 - 1.0 / 4.0 * cos(x3);
	out[2] = -x6 * (v * x5 + 2);
	return m.normalized();
}

ChristoffelSymbols MobiusStrip::christoffelSymbols(f32 u, f32 v) const {
	f32 x0 = (1.0 / 2.0) * u;
	f32 x1 = sin(x0);
	f32 x2 = pow(v, 2);
	f32 x3 = cos(x0);
	f32 x4 = 16 * v * x3 + 16;
	f32 x5 = 4 * pow(x3, 2);
	f32 x6 = (v * x5 + v + 8 * x3) / (x2 * x5 + x2 + x4);
	Mat2 x = Mat2::identity, y = Mat2::identity;
	f32* out = x.data();
	out[0] = -v * (v * sin(u) + 4 * x1) / (2 * x2 * cos(u) + 3 * x2 + x4);
	out[1] = x6;
	out[2] = x6;
	out[3] = 0;
	out = y.data();
	out[0] = v * pow(x1, 2) - 5.0 / 4.0 * v - 2 * x3;
	out[1] = 0;
	out[2] = 0;
	out[3] = 0;
	return { .x = x, .y = y };
}

