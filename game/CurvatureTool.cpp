#include "CurvatureTool.hpp"
#include <engine/Math/Color.hpp>
#include <engine/Input/Input.hpp>

void CurvatureTool::update(
	std::vector<MeshIntersection>& intersections, 
	const Surfaces& surfaces, 
	Renderer& renderer) {

	auto point = surfaces.position(pointUv);
	sortIntersectionsByDistanceToCamera(intersections);
	if (Input::isMouseButtonDown(MouseButton::LEFT) && checkIfPointGotGrabbed(point, intersections)) {
		grabbed = true;
	}
	if (Input::isMouseButtonHeld(MouseButton::LEFT) && grabbed) {
		updateGrabbedPoint(pointUv, point, intersections);
	}
	point = surfaces.position(pointUv);

	renderer.sphere(point, 0.015f, Color3::RED);

	const auto curvatures = surfaces.principalCurvatures(pointUv);
	const auto tangentU = surfaces.tangentU(pointUv);
	const auto tangentV = surfaces.tangentV(pointUv);
	const auto normal = surfaces.normal(pointUv).normalized();
	//renderer.circleArc()
	for (i32 i = 0; i < 2; i++) {
		const auto c = curvatures.curvature[i];
		const auto d = curvatures.direction[i];
		const auto dir = (d.x * tangentU + d.y * tangentV).normalized();
		const auto radius = 1.0f / c;
		renderer.circleArc(point + normal * radius, normal, dir, radius);
		//const auto dir = 
	}
}
