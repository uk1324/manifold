#include "Cycloid.hpp"
#define _USE_MATH_DEFINES
#include <math.h>

Vec3 Cycloid::position(f32 t) const {
	Vec3 m(0.0f);
	f32* out = m.data();
	out[0] = r * (t - sin(t));
	out[1] = r * (1 - cos(t));
	out[2] = 0;
	return m;
}

Vec3 Cycloid::tangent(f32 t) const {
	Vec3 m(0.0f);
	f32* out = m.data();
	f32 x0 = cos(t);
	out[0] = (1.0 / 2.0) * sqrt(2 - 2 * x0);
	out[1] = (1.0 / 2.0) * M_SQRT2 * sin(t) / sqrt(1 - x0);
	out[2] = 0;
	return m;
}

Vec3 Cycloid::normal(f32 t) const {
	Vec3 m(0.0f);
	f32* out = m.data();
	f32 x0 = sin(t);
	f32 x1 = M_SQRT2;
	f32 x2 = cos(t) - 1;
	f32 x3 = -x2;
	out[0] = (1.0 / 2.0) * x0 * x1 / sqrt(x3);
	out[1] = x1 * ((1.0 / 2.0) * pow(x0, 2) + x2) / pow(x3, 3.0 / 2.0);
	out[2] = 0;
	return m;
}

Vec3 Cycloid::binormal(f32 t) const {
	Vec3 m(0.0f);
	f32* out = m.data();
	out[0] = 0;
	out[1] = 0;
	out[2] = -1;
	return m;
}

f32 Cycloid::curvature(f32 t) const {
	f32 out;
	out = fabs(cos(t) - 1) / (r * pow(2 - 2 * cos(t), 3.0 / 2.0));
	return out;
}

f32 Cycloid::torsion(f32 t) const {
	f32 out;
	out = 0;
	return out;
}