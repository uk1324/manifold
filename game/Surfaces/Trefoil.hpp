#pragma once

#include "ChristoffelSymbols.hpp"
#include "Connectivity.hpp"
#include <engine/Math/Angles.hpp>
#include <engine/Math/Vec3.hpp>

struct Trefoil {
	f32 r, R;
	Vec3 position(f32 u, f32 v) const;
	Vec3 tangentU(f32 u, f32 v) const;
	Vec3 tangentV(f32 u, f32 v) const;
	Vec3 xUu(f32 u, f32 v) const;
	Vec3 xVv(f32 u, f32 v) const;
	Vec3 xUv(f32 u, f32 v) const;
	Mat2 firstFundamentalForm(f32 u, f32 v) const;
	Mat2 secondFundamentalForm(f32 u, f32 v) const;
	Vec3 normal(f32 u, f32 v) const;
	ChristoffelSymbols christoffelSymbols(f32 u, f32 v) const;
	f32 curvature(f32 u, f32 v) const;

	static constexpr auto uConnectivity = SquareSideConnectivity::NORMAL;
	static constexpr auto vConnectivity = SquareSideConnectivity::NORMAL;

	static constexpr f32 uMin = 0.0f;
	static constexpr f32 uMax = TAU<f32>;
	static constexpr f32 vMin = 0.0f;
	static constexpr f32 vMax = TAU<f32>;
};

Vec3 trefoilCurve(f32 t);