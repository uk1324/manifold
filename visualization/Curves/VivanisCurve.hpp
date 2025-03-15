#pragma once

#include <engine/Math/Vec3.hpp>

/*
https://en.wikipedia.org/wiki/Viviani%27s_curve
Intersection of sphere and cyllinder
It can be projected into various lemniscates on the plane.
*/
struct VivanisCurve {
	f32 r;
	Vec3 position(f32 t) const;
	Vec3 tangent(f32 t) const;
	Vec3 normal(f32 t) const;
	Vec3 binormal(f32 t) const;
	f32 curvature(f32 t) const;
	f32 torsion(f32 t) const;
};