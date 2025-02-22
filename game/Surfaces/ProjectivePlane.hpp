#pragma once

#include "GenerateParametrization.hpp"
#include <engine/Math/Angles.hpp>

struct ProjectivePlane : GenerateParametrization<ProjectivePlane> {
	Vec3 position(f32 u, f32 v) const;

	static constexpr auto uConnectivity = SquareSideConnectivity::REVERSED;
	static constexpr auto vConnectivity = SquareSideConnectivity::REVERSED;

	static constexpr f32 uMin = 0.0f;
	static constexpr f32 uMax = 1.0f;
	static constexpr f32 vMin = 0.0f;
	static constexpr f32 vMax = TAU<f32>;
};
