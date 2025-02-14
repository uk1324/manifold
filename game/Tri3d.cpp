#include "Tri3d.hpp"

Vec3 triCenter(Vec3 v0, Vec3 v1, Vec3 v2) {
	return (v0 + v1 + v2) / 3.0f;
}

Vec3 triCenter(const Vec3* v) {
	return triCenter(v[0], v[2], v[1]);
}
