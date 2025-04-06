#include "Bezier.hpp"

Vec3 derivBezier(const Vec3 P[4], f32 t) {
	return -3 * (1 - t) * (1 - t) * P[0] +
		(3 * (1 - t) * (1 - t) - 6 * t * (1 - t)) * P[1] +
		(6 * t * (1 - t) - 3 * t * t) * P[2] +
		3 * t * t * P[3];
}

Vec3 evalBezierCurve(const Vec3 P[4], f32 t) {
	f32 b0 = (1 - t) * (1 - t) * (1 - t);
	f32 b1 = 3 * t * (1 - t) * (1 - t);
	f32 b2 = 3 * t * t * (1 - t);
	f32 b3 = t * t * t;
	return P[0] * b0 + P[1] * b1 + P[2] * b2 + P[3] * b3;
}