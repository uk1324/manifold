#pragma once

#include "RectParametrization.hpp"
#include <engine/Math/Angles.hpp>

struct Sphere {
	Vec3 position(f32 u, f32 v) const;
	Vec3 tangentU(f32 u, f32 v) const;
	Vec3 tangentV(f32 u, f32 v) const;
	Vec3 normal(f32 u, f32 v) const;
	ChristoffelSymbols christoffelSymbols(f32 u, f32 v) const;
	f32 curvature(f32 u, f32 v) const;
	PrincipalCurvatures principalCurvatures(f32 u, f32 v) const;
	f32 r;

	static constexpr auto uConnectivity = SquareSideConnectivity::NORMAL;
	static constexpr auto vConnectivity = SquareSideConnectivity::NORMAL;

	static constexpr f32 uMin = 0.0f;
	static constexpr f32 uMax = PI<f32>;
	static constexpr f32 vMin = 0.0f;
	static constexpr f32 vMax = TAU<f32>;
};