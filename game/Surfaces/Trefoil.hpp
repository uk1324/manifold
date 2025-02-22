#pragma once

#include "GenerateParametrization.hpp"
#include <engine/Math/Angles.hpp>

struct Trefoil : GenerateParametrization<Trefoil> {
	f32 r, R;
	Vec3 position(f32 u, f32 v) const;

	static constexpr auto uConnectivity = SquareSideConnectivity::NORMAL;
	static constexpr auto vConnectivity = SquareSideConnectivity::NORMAL;

	static constexpr f32 uMin = 0.0f;
	static constexpr f32 uMax = TAU<f32>;
	static constexpr f32 vMin = 0.0f;
	static constexpr f32 vMax = TAU<f32>;
};

Vec3 trefoilCurve(f32 t);