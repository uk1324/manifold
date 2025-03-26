#pragma once

#include <game/GameRenderer.hpp>
#include <game/FpsCamera3d.hpp>
#include <game/Polyhedra.hpp>

struct Visualization {
	Visualization();

	void update();

	void renderPolygonSoup(const PolygonSoup& polygonSoup);

	GameRenderer renderer;
	FpsCamera3d camera;
};