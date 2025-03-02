#include "Helix.hpp"

Vec3 Helix::position(f32 t) const {
	Vec3 m(0.0f);
	f32* out = m.data();
	out[0] = a * cos(t);
	out[1] = a * sin(t);
	out[2] = b * t;
	return m;
}

Vec3 Helix::tangent(f32 t) const {
	Vec3 m(0.0f);
	f32* out = m.data();
	f32 x0 = pow(pow(a, 2) + pow(b, 2), -1.0 / 2.0);
	f32 x1 = a * x0;
	out[0] = -x1 * sin(t);
	out[1] = x1 * cos(t);
	out[2] = b * x0;
	return m;
}

Vec3 Helix::normal(f32 t) const {
	Vec3 m(0.0f);
	f32* out = m.data();
	out[0] = -cos(t);
	out[1] = -sin(t);
	out[2] = 0;
	return m;
}

Vec3 Helix::binormal(f32 t) const {
	Vec3 m(0.0f);
	f32* out = m.data();
	f32 x0 = pow(pow(a, 2) + pow(b, 2), -1.0 / 2.0);
	f32 x1 = b * x0;
	out[0] = x1 * sin(t);
	out[1] = -x1 * cos(t);
	out[2] = a * x0;
	return m;
}

f32 Helix::curvature(f32 t) const {
	f32 out;
	out = a / (pow(a, 2) + pow(b, 2));
	return out;
}

f32 Helix::torsion(f32 t) const {
	f32 out;
	out = b / (pow(a, 2) + pow(b, 2));
	return out;
}