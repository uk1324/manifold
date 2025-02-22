#pragma once

#include <game/Curves/Helix.hpp>

struct Curves {
	enum class Type {
		HELIX
	};
	Type type = Type::HELIX;

	Vec3 position(f32 t) const;
	Vec3 tangent(f32 t) const;
	Vec3 normal(f32 t) const;
	Vec3 binormal(f32 t) const;
	f32 curvature(f32 t) const;
	f32 torsion(f32 t) const;

	Helix helix{
		.a = 1.0f,
		.b = 1.0f
	};
};