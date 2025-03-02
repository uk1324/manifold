#pragma once

#include <engine/Math/Vec3.hpp>

struct TrefoilKnot {
	f32 a;
	f32 b;
	Vec3 position(f32 t) const;
	Vec3 tangent(f32 t) const;
	Vec3 normal(f32 t) const;
	Vec3 binormal(f32 t) const;
	f32 curvature(f32 t) const;
	f32 torsion(f32 t) const;
};