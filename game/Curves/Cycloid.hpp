#pragma once

#include <engine/Math/Vec3.hpp>

struct Cycloid {
	f32 r = 1.0f;
	Vec3 position(f32 t) const;
	Vec3 tangent(f32 t) const;
	Vec3 normal(f32 t) const;
	Vec3 binormal(f32 t) const;
	f32 curvature(f32 t) const;
	f32 torsion(f32 t) const;
};