#pragma once

#include "RectParametrization.hpp"
#include <engine/Math/Angles.hpp>

struct MobiusStrip {
	Vec3 position(f32 u, f32 v) const;
	Vec3 tangentU(f32 u, f32 v) const;
	Vec3 tangentV(f32 u, f32 v) const;
	Vec3 xUu(f32 u, f32 v) const;
	Vec3 xVv(f32 u, f32 v) const;
	Vec3 xUv(f32 u, f32 v) const;
	Vec3 normal(f32 u, f32 v) const;
	ChristoffelSymbols christoffelSymbols(f32 u, f32 v) const;
	Mat2 firstFundamentalForm(f32 u, f32 v) const;
	Mat2 secondFundamentalForm(f32 u, f32 v) const;
	f32 curvature(f32 u, f32 v) const;
	PrincipalCurvatures principalCurvatures(f32 u, f32 v) const;

	static constexpr f32 uMin = 0;
	static constexpr f32 uMax = TAU<f32>;
	static constexpr f32 vMin = -1.0f;
	static constexpr f32 vMax = 1.0f;

	static constexpr auto uConnectivity = SquareSideConnectivity::REVERSED;
	static constexpr auto vConnectivity = SquareSideConnectivity::NONE;
};