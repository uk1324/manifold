#include "VivanisCurve.hpp"

Vec3 VivanisCurve::position(f32 t) const {
	Vec3 m(0.0f);
	f32* out = m.data();
	f32 x0 = cos(t);
	f32 x1 = r * sin(t);
	out[0] = r * pow(x0, 2);
	out[1] = x0 * x1;
	out[2] = x1;
	return m;
}

Vec3 VivanisCurve::tangent(f32 t) const {
	Vec3 m(0.0f);
	f32* out = m.data();
	f32 x0 = 2 * t;
	f32 x1 = cos(t);
	f32 x2 = pow(pow(x1, 2) + 1, -1.0 / 2.0);
	out[0] = -x2 * sin(x0);
	out[1] = x2 * cos(x0);
	out[2] = x1 * x2;
	return m;
}

Vec3 VivanisCurve::normal(f32 t) const {
	Vec3 m(0.0f);
	f32* out = m.data();
	f32 x0 = sin(t);
	f32 x1 = pow(x0, 2);
	f32 x2 = (x1 - 2) / (sqrt(8 - 3 * x1) * pow(pow(cos(t), 2) + 1, 3.0 / 2.0));
	out[0] = 2 * x2 * (pow(x0, 4) - 4 * x1 + 2);
	out[1] = (1.0 / 4.0) * x2 * (12 * sin(2 * t) + sin(4 * t));
	out[2] = x0 * x2;
	return m;
}

Vec3 VivanisCurve::binormal(f32 t) const {
	Vec3 m(0.0f);
	f32* out = m.data();
	f32 x0 = sin(t);
	f32 x1 = pow(x0, 2);
	f32 x2 = pow(8 - 3 * x1, -1.0 / 2.0);
	f32 x3 = cos(t);
	out[0] = x0 * x2 * (3 - 2 * x1);
	out[1] = -2 * pow(x3, 3) / sqrt(3 * pow(x3, 2) + 5);
	out[2] = 2 * x2;
	return m;
}

f32 VivanisCurve::curvature(f32 t) const {
	f32 out;
	out = sqrt(pow(2 * pow(sin(t), 2) - 3, 2) * pow(sin(t), 2) + 4 * pow(cos(t), 6) + 4) / (r * pow(pow(cos(t), 2) + 1, 3.0 / 2.0));
	return out;
}

f32 VivanisCurve::torsion(f32 t) const {
	f32 out;
	out = 6 * cos(t) / (r * (pow(2 * pow(sin(t), 2) - 3, 2) * pow(sin(t), 2) + 4 * pow(cos(t), 6) + 4));
	return out;
}


