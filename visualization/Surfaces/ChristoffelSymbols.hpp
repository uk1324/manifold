#pragma once

#include <engine/Math/Mat2.hpp>

struct ChristoffelSymbols {
	Mat2 x;
	Mat2 y;
};

// The uv tangents of the surface can also be called partial velocities.