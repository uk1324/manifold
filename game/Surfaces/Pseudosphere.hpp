#pragma once

#include "ChristoffelSymbols.hpp"
#include "Connectivity.hpp"
#include <engine/Math/Vec3.hpp>
#include <engine/Math/Constants.hpp>

struct Pseudosphere {
	Vec3 position(f32 u, f32 v);
	Vec3 tangentU(f32 u, f32 v);
	Vec3 tangentV(f32 u, f32 v);
	Vec3 normal(f32 u, f32 v);
	ChristoffelSymbols christoffelSymbols(f32 u, f32 v);

	f32 r;

	static constexpr auto uConnectivity = SquareSideConnectivity::NONE;
	static constexpr auto vConnectivity = SquareSideConnectivity::NORMAL;

	// The christoffel symbols are signular u = 0.
	static constexpr f32 uMin = 0.0f;
	static constexpr f32 uMax = 5.0f;
	static constexpr f32 vMin = 0.0f;
	static constexpr f32 vMax = TAU<f32>;
};