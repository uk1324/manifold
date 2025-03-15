#include "PlotUtils.hpp"
#include <imgui/implot.h>

void plotVec2Scatter(const char* label, const std::vector<Vec2>& points) {
	const auto pointsData = reinterpret_cast<const float*>(points.data());
	ImPlot::PlotScatter(label, pointsData, pointsData + 1, points.size(), 0, 0, sizeof(Vec2));
}