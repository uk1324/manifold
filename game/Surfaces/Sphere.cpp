#include "Sphere.hpp"

// https://trecs.se/sphere.php
// theta <-> u
// phi <-> v

// https://math.stackexchange.com/questions/3361122/formula-for-parametrization-of-two-sphere-that-works-at-every-point
// "Therefore no chart f: U -> S^2 can be surjective"
// Is this wrong? Couldn't you use the unwrapped cube parametrization?
// The issue with the cube parametrization is that the vector field is discontinous so the theorem doesn't apply.

// You could also probably use the extended sphere 

// Spherical coordinates inverse
// https://math.stackexchange.com/questions/320633/inverse-jacobian-matrix-of-spherical-coordinates
Vec3 Sphere::position(f32 u, f32 v) const {
	return Vec3(
		r * sin(u) * cos(v),
		r * sin(u) * sin(v),
		r * cos(u)
	);
}

Vec3 Sphere::tangentU(f32 u, f32 v) const {
	return Vec3(
		r * cos(u) * cos(v),
		r * cos(u) * sin(v),
		-r * sin(u)
	);
}

Vec3 Sphere::tangentV(f32 u, f32 v) const {
	return Vec3(
		-r * sin(u) * sin(v),
		r * sin(u) * cos(v),
		0
	);
}

Vec3 Sphere::normal(f32 u, f32 v) const {
	return position(u, v).normalized();
	//return Vec3(
	//	sin(u) * cos(v),
	//	sin(u) * sin(v),
	//	cos(u)
	//).normalized();
}

// The y symbols are undefined on the line y = 0. This is because the parametrization maps this whole line of points into a single point on the line.
// Not sure if this makes sense, but this also means that the uniqueness condition (uniqueness condition of what, in the parametrization or on the manifold), doesn't hold. If it were to hold then 
ChristoffelSymbols Sphere::christoffelSymbols(f32 u, f32 v) const {
	return {
		.x = Mat2(Vec2(0.0f, 0.0f), Vec2(0.0f, -0.5 * sin(2.0f * u))),
		.y = Mat2(Vec2(0.0f, 1.0f / tan(u)), Vec2(1.0f / tan(u), 0.0f))
	};
}
