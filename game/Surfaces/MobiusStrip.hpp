#pragma once

#include "ChristoffelSymbols.hpp"
#include "Connectivity.hpp"
#include <engine/Math/Vec3.hpp>
#include <engine/Math/Constants.hpp>

struct MobiusStrip {
	Vec3 position(f32 u, f32 v);
	Vec3 tangentU(f32 u, f32 v);
	Vec3 tangentV(f32 u, f32 v);
	Vec3 normal(f32 u, f32 v);
	ChristoffelSymbols christoffelSymbols(f32 u, f32 v);

	static constexpr f32 uMin = 0;
	static constexpr f32 uMax = TAU<f32>;
	static constexpr f32 vMin = -1.0f;
	static constexpr f32 vMax = 1.0f;

	static constexpr auto uConnectivity = SquareSideConnectivity::REVERSED;
	static constexpr auto vConnectivity = SquareSideConnectivity::NONE;
};