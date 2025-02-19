#pragma once

#include <game/Utils.hpp>
#include <game/Surfaces.hpp>
#include <game/Renderer.hpp>

struct CurvatureTool {
	Vec2 pointUv;
	bool grabbed = false;

	void update(
		std::vector<MeshIntersection>& intersections,
		const Surfaces& surfaces,
		Renderer& renderer);
};