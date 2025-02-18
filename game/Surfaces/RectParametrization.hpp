#pragma once

#include "ChristoffelSymbols.hpp"
#include "Connectivity.hpp"
#include <engine/Math/Vec3.hpp>

template<typename Parametrization>
concept RectParametrization = requires(Parametrization surface, f32 u, f32 v) {
	{ surface.normal(u, v) } -> std::convertible_to<Vec3>;
	{ surface.tangentU(u, v) } -> std::convertible_to<Vec3>;
	{ surface.tangentV(u, v) } -> std::convertible_to<Vec3>;
	{ surface.christoffelSymbols(u, v) } -> std::convertible_to<ChristoffelSymbols>;
	{ surface.uConnectivity } -> std::convertible_to<SquareSideConnectivity>;
	{ surface.vConnectivity } -> std::convertible_to<SquareSideConnectivity>;
	{ surface.uMin } -> std::convertible_to<f32>;
	{ surface.uMax } -> std::convertible_to<f32>;
	{ surface.vMin } -> std::convertible_to<f32>;
	{ surface.vMax } -> std::convertible_to<f32>;
};

Vec3 normal(const RectParametrization auto& s, Vec2 uv);

Vec3 normal(const RectParametrization auto& s, Vec2 uv) {
	return s.normal(uv.x, uv.y);
}