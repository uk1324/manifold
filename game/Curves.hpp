#pragma once

#include <game/Curves/Helix.hpp>
#include <game/Curves/VivanisCurve.hpp>
#include <game/Curves/Cycloid.hpp>

// The evolute of a cycloid is an another cycloid.
struct Curves {
	enum class Type {
		HELIX,
		VIVANIS_CURVE,
		CYCLOID,
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
	VivanisCurve vivanisCurve{
		.r = 2.0f
	};
	Cycloid cycloid{
		.r = 2.0f
	};
};