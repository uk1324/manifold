#pragma once

#include <game/SurfaceVisualization.hpp>
#include <game/CurveVisualization.hpp>

struct MainLoop {
	MainLoop();

	void update();

	enum class Mode {
		SURFACE, CURVE
	};
	SurfaceVisualization surfaceVisualization;
	CurveVisualization curveVisualization;
	Mode mode = Mode::CURVE;

	Renderer renderer;
};