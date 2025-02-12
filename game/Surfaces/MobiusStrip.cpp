#include "MobiusStrip.hpp"

Vec3 MobiusStrip::position(f32 u, f32 v) {
	return Vec3(
		(1 + 0.5f * v * cos(0.5 * u)) * cos(u),
		(1 + 0.5f * v * cos(0.5 * u)) * sin(u),
		0.5 * v * sin(0.5 * u)
	);
}
