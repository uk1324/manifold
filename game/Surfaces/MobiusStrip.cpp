#include "MobiusStrip.hpp"

// https://trecs.se/M%C3%B6biusStrip.php

Vec3 MobiusStrip::position(f32 u, f32 v) {
	return Vec3(
		(v*cos(u/2)/2 + 1)*cos(u),
		(v*cos(u/2)/2 + 1)*sin(u),
		v*sin(u/2)/2
	)
}

Vec3 MobiusStrip::tangentU(f32 u, f32 v) {
	return Vec3(
		-v*sin(u/2)*cos(u)/4 - (v*cos(u/2) + 2)*sin(u)/2,
		-v*sin(u/2)*sin(u)/4 + (v*cos(u/2) + 2)*cos(u)/2,
		v*cos(u/2)/4
	)
}

Vec3 MobiusStrip::tangentV(f32 u, f32 v) {
	return Vec3(
		cos(u/2)*cos(u)/2,
		sin(u)*cos(u/2)/2,
		sin(u/2)/2
	)
}

Vec3 MobiusStrip::normal(f32 u, f32 v) {
	return Vec3(
		v*sin(u)*cos(u)/8 - v*sin(u)/8 - sin(u/2)/4 + sin(3*u/2)/4,
		v*cos(u/2)**2*cos(u)/8 - 3*v*cos(u)**2/16 + v*cos(u)/16 + v/8 + cos(u/2)/4 - cos(3*u/2)/4,
		-(v*cos(u/2) + 2)*cos(u/2)/4
	)
}

Mat2 MobiusStrip::christoffelSymbols(f32 u, f32 v) {
	return {
		.x = Mat2(
			Vec2(-v*(v*sin(u) + 4*sin(u/2))/(2*v*v*cos(u) + 3*v**2 + 16*v*cos(u/2) + 16), (4*v*cos(u/2)**2 + v + 8*cos(u/2))/(4*v*v*cos(u/2)**2 + v**2 + 16*v*cos(u/2) + 16)),
			Vec2((4*v*cos(u/2)**2 + v + 8*cos(u/2))/(4*v**2*cos(u/2)**2 + v**2 + 16*v*cos(u/2) + 16), 0)
		)
		.y = Mat2(
			Vec2(v*sin(u/2)**2 - 5*v/4 - 2*cos(u/2), 0),
			Vec2(0, 0)
		)
	}
}


