#pragma once

#include <game/Surfaces/Helicoid.hpp>
#include <game/Surfaces/Pseudosphere.hpp>
#include <game/Surfaces/Cone.hpp>
#include <game/Surfaces/Torus.hpp>
#include <game/Surfaces/Trefoil.hpp>
#include <game/Surfaces/MobiusStrip.hpp>
#include <game/Surfaces/Sphere.hpp>

struct Surfaces {
	Torus torus{ .r = 0.4f, .R = 1.0f };
	//Torus torus{ .r = 0.4f, .R = 2.0f };
	Trefoil trefoil{ .r = 0.4f, .R = 2.0f };
	Helicoid helicoid{ .uMin = -PI<f32>, .uMax = PI<f32>, .vMin = -5.0f, .vMax = 5.0f };
	MobiusStrip mobiusStrip;
	Pseudosphere pseudosphere{ .r = 2.0f };
	Cone cone{
		.a = 1.0f,
		.b = 1.0f,
		.uMin = -2.0f,
		.uMax = 2.0f,
	};
	Sphere sphere{ .r = 1.0f };

	// Not using a sum type, because the settings should remain.
	enum class Type {
		TORUS,
		TREFOIL,
		HELICOID,
		MOBIUS_STRIP,
		PSEUDOSPHERE,
		CONE,
		SPHERE,
	};

	Vec3 position(Vec2 uv) const;
	Vec3 normal(Vec2 uv) const;
	Vec3 tangentU(Vec2 uv) const;
	Vec3 tangentV(Vec2 uv) const;
	PrincipalCurvatures principalCurvatures(Vec2 uv) const;

	Type selected = Type::TORUS;
};
const char* surfaceNameStr(Surfaces::Type surface);