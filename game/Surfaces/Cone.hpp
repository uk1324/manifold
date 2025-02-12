#pragma once
#include "ChristoffelSymbols.hpp"
#include "Connectivity.hpp"
#include <engine/Math/Vec3.hpp>
#include <engine/Math/Constants.hpp>

struct Cone {
	Vec3 position(f32 u, f32 v);
	Vec3 tangentU(f32 u, f32 v);
	Vec3 tangentV(f32 u, f32 v);
	Vec3 normal(f32 u, f32 v);
	ChristoffelSymbols christoffelSymbols(f32 u, f32 v);

	static constexpr auto uConnectivity = SquareSideConnectivity::NONE;
	static constexpr auto vConnectivity = SquareSideConnectivity::NORMAL;

	f32 a, b;
	f32 uMin;
	f32 uMax;
	static constexpr f32 vMin = 0.0f;
	static constexpr f32 vMax = TAU<f32>;
};