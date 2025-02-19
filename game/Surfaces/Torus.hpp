#pragma once

#include "ChristoffelSymbols.hpp"
#include "Connectivity.hpp"
#include <engine/Math/Angles.hpp>
#include <engine/Math/Vec3.hpp>

struct Torus {
	f32 r, R;
	Vec3 position(f32 u, f32 v) const;
	Vec3 tangentU(f32 u, f32 v) const;
	Vec3 tangentV(f32 u, f32 v) const;
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

// https://trecs.se/torus.php
Vec3 torusPosition(f32 u, f32 v, f32 r, f32 R);
Vec3 torusTangentU(f32 u, f32 v, f32 r, f32 R);
Vec3 torusTangentV(f32 u, f32 v, f32 r, f32 R);
Vec3 torusNormal(f32 u, f32 v, f32 r, f32 R);
ChristoffelSymbols torusChristoffelSymbols(f32 u, f32 v, f32 r, f32 R);