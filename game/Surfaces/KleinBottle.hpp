#pragma once

#include "GenerateParametrization.hpp"
#include <engine/Math/Angles.hpp>

struct KleinBottle : GenerateParametrization<KleinBottle> {
	Vec3 position(f32 u, f32 v) const;

	static constexpr auto uConnectivity = SquareSideConnectivity::REVERSED;
	static constexpr auto vConnectivity = SquareSideConnectivity::NORMAL;

	//static constexpr f32 uMin = 0.0f;
	//static constexpr f32 uMax = TAU<f32>;
	//static constexpr f32 vMin = 0.0f;
	////static constexpr f32 vMax = TAU<f32>;
	//static constexpr f32 vMax = 2.0f * TAU<f32>;

	static constexpr f32 uMin = 0.0f;
	static constexpr f32 uMax = PI<f32>;
	static constexpr f32 vMin = 0.0f;
	//static constexpr f32 vMax = TAU<f32>;
	static constexpr f32 vMax = TAU<f32>;
	//static constexpr f32 uMin = 0.0f;
	//static constexpr f32 uMax = TAU<f32>;
	//static constexpr f32 vMin = 0.0f;
	////static constexpr f32 vMax = TAU<f32>;
	//static constexpr f32 vMax = 2.0f * TAU<f32>;
};
