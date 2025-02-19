#pragma once

#include <game/Surfaces.hpp>
#include <game/Surfaces/RectParametrization.hpp>
#include <game/Utils.hpp>
#include <game/Renderer.hpp>
#include <vector>

struct GeodesicTool {
	Vec2 initialPositionUv = Vec2(0.1f);
	Vec2 initialVelocityUv = Vec2(0.0f, 1.0f);
	enum class Grabbed {
		NONE,
		POSITION,
		VELOCITY
	};
	Grabbed grabbed = Grabbed::NONE;

	void update(
		Vec3 cameraPosition, 
		Vec3 cameraForward, 
		std::vector<MeshIntersection>& intersections,
		const Surfaces& surfaces,
		Renderer& renderer);
	void integrateGeodesic(const RectParametrization auto& surface, Renderer& renderer);
};