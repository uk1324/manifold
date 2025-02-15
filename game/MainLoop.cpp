#include "MainLoop.hpp"
#include <glad/glad.h>
#include <Dbg.hpp>
#include <engine/Math/Color.hpp>
#include <Timer.hpp>
#include <engine/Math/OdeIntegration/RungeKutta4.hpp>
#include <engine/Math/Interpolation.hpp>
#include <game/PlotUtils.hpp>
#include <engine/Math/Mat2.hpp>
#include <engine/Math/Angles.hpp>
#include <engine/Math/Constants.hpp>
#include <engine/Window.hpp>
#include <engine/Input/Input.hpp>
#include <game/Surfaces/Helicoid.hpp>
#include <game/Surfaces/Pseudosphere.hpp>
#include <game/Surfaces/Cone.hpp>
#include <game/Surfaces/Torus.hpp>
#include <game/Surfaces/Trefoil.hpp>
#include <game/Surfaces/MobiusStrip.hpp>
#include <game/Surfaces/Sphere.hpp>
#include <imgui/implot.h>
#include <gfx/ShaderManager.hpp>
#include <game/Tri3d.hpp>
#include <game/RayIntersection.hpp>

template<typename T>
void getTriangle(const std::vector<T>& values, const std::vector<i32>& indices, T* triangle, i32 triangleIndex) {
	for (i32 i = 0; i < 3; i++) {
		const auto index = indices[triangleIndex * 3 + i];
		triangle[i] = values[index];
	}
};

const auto dt = 1.0f / 60.0f;

void generateParametrizationOfRectangle(
	MainLoop::Surface& surface,
	auto position,
	auto normal,
	f32 uMin, f32 uMax, f32 vMin, f32 vMax) {
	//const auto size = 100;
	const auto size = 50;
	for (i32 vi = 0; vi <= size; vi++) {
		for (i32 ui = 0; ui <= size; ui++) {
			const auto ut = f32(ui) / size;
			const auto vt = f32(vi) / size;
			const auto u = lerp(uMin, uMax, ut);
			const auto v = lerp(vMin, vMax, vt);
			const auto p = position(u, v);
			const auto n = normal(u, v);
			surface.addVertex(p, n, Vec2(u, v), Vec2(ut, vt));
		}
	}

	auto index = [&size](i32 ui, i32 vi) {
		//// Wrap aroud
		//if (ui == size) { ui = 0; }
		//if (vi == size) { vi = 0; }

		return vi * (size + 1) + ui;
	};
	for (i32 vi = 0; vi < size; vi++) {
		for (i32 ui = 0; ui < size; ui++) {
			const auto i0 = index(ui, vi);
			const auto i1 = index(ui + 1, vi);
			const auto i2 = index(ui + 1, vi + 1);
			const auto i3 = index(ui, vi + 1);
			indicesAddQuad(surface.indices, i0, i1, i2, i3);
		}
	}

	for (i32 i = 0; i < surface.triangleCount(); i++) {
		Vec3 triangle[3];
		getTriangle(surface.positions, surface.indices, triangle, i);
		surface.triangleCenters.push_back(triCenter(triangle));
	}
}

Torus surface{ .r = 0.4f, .R = 2.0f };
//Trefoil surface{ .r = 0.4f, .R = 2.0f };
//Helicoid surface{ .uMin = -PI<f32>, .uMax = PI<f32>, .vMin = -5.0f, .vMax = 5.0f };
//MobiusStrip surface;
//Pseudosphere surface{ .r = 2.0f };
//Cone surface{
//	.a = 1.0f,
//	.b = 1.0f,
//	.uMin = -2.0f,
//	.uMax = 2.0f,
//};
//Sphere surface{ .r = 1.0f };

MainLoop::MainLoop()
	: renderer(Renderer::make()) {

	generateParametrizationOfRectangle(
		surfaceMesh,
		[&](f32 u, f32 v) { return surface.position(u, v); },
		[&](f32 u, f32 v) { return surface.normal(u, v); },
		surface.uMin, surface.uMax,
		surface.vMin, surface.vMax
	);
	for (i32 i = 0; i < surfaceMesh.triangleCount(); i++) {
		sortedTriangles.push_back(i);
	}

	Window::disableCursor();
}

void MainLoop::update() {
	ShaderManager::update();

	if (Input::isKeyDown(KeyCode::ESCAPE)) {
		Window::toggleCursor();
	}

	if (Input::isKeyDown(KeyCode::X)) {
		Window::close();
	}

	//auto renderLine = [this](Vec3 a, Vec3 b) {
	//	i32 circleVertexCount = 50;
	//	const auto beginIndex = renderer.trianglesVertices.size();
	//	for (i32 i = 0; i < circleVertexCount; i++) {
	//		const auto t = f32(i) / f32(circleVertexCount);
	//		const auto a = lerp(0.0f, TAU<f32>, t);
	//		const auto translation = normal * cos(a) + binormal * sin(a);
	//		//const auto vertexBottom = 
	//	}
	//};

	if (Input::isKeyDown(KeyCode::TAB)) {
		if (cameraMode == CameraMode::ON_SURFACE) {
			cameraMode = CameraMode::IN_SPACE;
		} else if (cameraMode == CameraMode::IN_SPACE) {
			cameraMode = CameraMode::ON_SURFACE;
		}
	}

	auto view = Mat4::identity;
	Vec3 cameraPosition(0.0f);
	switch (cameraMode) {
		using enum CameraMode;
	case ON_SURFACE: {
		view = surfaceCamera.update(surface, dt);
		cameraPosition = surfaceCamera.cameraPosition(surface);
		break;
	}

	case IN_SPACE: {
		fpsCamera.update(dt);
		view = fpsCamera.viewMatrix();
		cameraPosition = fpsCamera.position;
		inSpaceUpdate(cameraPosition);
		break;
	}
		
	}

  	const auto aspectRatio = Window::aspectRatio();
	const auto projection = Mat4::perspective(PI<f32> / 2.0f, aspectRatio, 0.1f, 1000.0f);
	const auto swaxpYZ = Mat4(Mat3(Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f), Vec3(0.0f, 1.0f, 0.0f)));
	renderer.transform = projection * view;
	renderer.view = view;

	//const auto n = 200;
	//for (i32 i = 0; i < n; i++) {
	//	auto t0 = f32(i) / f32(n) * TAU<f32>;
	//	auto t1 = f32(i + 1) / f32(n) * TAU<f32>;
	//	const auto v0 = trefoilCurve(t0);
	//	const auto v1 = trefoilCurve(t1);
	//	renderer.line(v0, v1, 0.02f, Color3::RED);
	//}
	/*static f32 t = 0.0f;
	t += dt;
	t = 0.0f;*/
	//renderer.line(Vec3(0.0f, 0.0f, 0.0f), fpsCamera.position - Vec3(0.0f, 0.5f, 0.0f), 0.05f, Color3::RED);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, i32(Window::size().x), i32(Window::size().y));

	const auto indicesOffset = i32(renderer.trianglesIndices.size());
	for (i32 i = 0; i < surfaceMesh.vertexCount(); i++) {
		renderer.addVertex(Vertex3Pnt{ 
			.position = surfaceMesh.positions[i], 
			.normal = surfaceMesh.normals[i], 
			.uv = surfaceMesh.uvts[i] 
		});
	}

	std::vector<f32> distances;
	for (i32 i = 0; i < surfaceMesh.triangleCenters.size(); i++) {
		distances.push_back(surfaceMesh.triangleCenters[i].distanceSquaredTo(cameraPosition));
	}
	const auto lessThan = [&](i32 a, i32 b) {
		return distances[a] > distances[b];
	};
	std::sort(sortedTriangles.begin(), sortedTriangles.end(), lessThan);


	for (i32 i = 0; i < surfaceMesh.triangleCount(); i++) {
		const auto index = indicesOffset + sortedTriangles[i] * 3;
		renderer.addTriangle(
			surfaceMesh.indices[index],
			surfaceMesh.indices[index + 1],
			surfaceMesh.indices[index + 2]
		);
	}

	//renderer.sphere(Vec3(0.0f, 0.0f, 5.0f), 0.1f, Color3::GREEN);

	renderer.renderCyllinders();
	renderer.renderHemispheres();
	renderer.renderCones();
	renderer.renderCircles();

	if (uvPositions.size() >= 2) {
		for (i32 i = 0; i < uvPositions.size() - 1; i++) {
			renderer.line(
				surface.position(uvPositions[i].x, uvPositions[i].y), 
				surface.position(uvPositions[i + 1].x, uvPositions[i + 1].y), 
				0.02f, 
				Color3::RED
			);
		}
	}

	//uvPositions.push_back(surfaceCamera.uvPosition);
	ImPlot::SetNextAxesLimits(-10, 80, -3, 20, ImGuiCond_Always);
	ImGui::Begin("plot");
	if (ImPlot::BeginPlot("plot", ImVec2(-1.0f, -1.0f), ImPlotFlags_Equal)) {
		ImPlot::SetupAxesLimits(surface.uMin, surface.uMax, surface.vMin, surface.vMax, ImPlotCond_Always);
		Vec2 forward = surfaceCamera.uvPosition + Vec2::oriented(surfaceCamera.uvForwardAngle) * 0.3f;
		f32 xs[] = { surfaceCamera.uvPosition.x, forward.x };
		f32 ys[] = { surfaceCamera.uvPosition.y, forward.y };
		ImPlot::PlotLine("arrow", xs, ys, 2);
		plotVec2Scatter("points", uvPositions);
		ImPlot::EndPlot();
	}
	ImGui::End();

	// Disabling depth writes was mentioned in real-time rendering. Not sure what it actually does.
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	renderer.renderTriangles(0.5f);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	renderer.gfx2d.disk(Vec2(0.0f), 0.015f, Color3::WHITE);
	renderer.gfx2d.drawDisks();
	glDisable(GL_BLEND);
}

void MainLoop::inSpaceUpdate(Vec3 cameraPosition) {
	const auto forward = fpsCamera.forward();

	struct Intersection {
		RayTriIntersection i;
		i32 triangleIndex;
		Vec2 uv;
	};
	std::vector<Intersection> intersections;
	for (i32 i = 0; i < surfaceMesh.triangleCount(); i++) {
		Vec3 vs[3];
		getTriangle(surfaceMesh.positions, surfaceMesh.indices, vs, i);
		const auto intersection = rayTriIntersection(cameraPosition, forward, vs);
		if (!intersection.has_value()) {
			continue;
		}
		Vec2 uvs[3];
		getTriangle(surfaceMesh.uvs, surfaceMesh.indices, uvs, i);
		const auto uv = barycentricInterpolate(intersection->barycentricCoordinates, uvs);
		intersections.push_back({ *intersection, i, uv });
	}
	std::ranges::sort(
		intersections,
		[](const Intersection& a, const Intersection& b) {
			return a.i.t < b.i.t;
		}
	);
	const auto initialVelocityVectorLength = 0.25f;
	const auto initialPositionPos = surface.position(initialPositionUv.x, initialPositionUv.y);
	const auto initialPositionTangentU = surface.tangentU(initialPositionUv.x, initialPositionUv.y);
	const auto initialPositionTangentV = surface.tangentV(initialPositionUv.x, initialPositionUv.y);
	const auto initialVelocityVectorEndPosition =
		initialPositionPos + 
		initialVelocityVectorLength * 
		(initialVelocityUv.x * initialPositionTangentU + initialVelocityUv.y * initialPositionTangentV).normalized();

	struct TangentPlanePosition {
		Vec3 inSpace;
		Vec2 inUvCoordinates;
	};
	std::optional<TangentPlanePosition> tangentPlaneIntersection;

	{
		const auto tangentPlaneNormal = cross(initialPositionTangentU, initialPositionTangentV);
		const auto intersectionT = rayPlaneIntersection(cameraPosition, forward, initialPositionPos, tangentPlaneNormal);
		if (intersectionT.has_value()) {
			const auto intersection = cameraPosition + *intersectionT * forward;
			Vec3 orthonormalTangentPlaneBasis[2]{
				initialPositionTangentU,
				cross(initialPositionTangentU, tangentPlaneNormal)
			};
			auto toOrthonormalBasis = [&orthonormalTangentPlaneBasis](Vec3 v) {
				return Vec2(
					dot(orthonormalTangentPlaneBasis[0], v),
					dot(orthonormalTangentPlaneBasis[1], v)
				);
			};
			// Instead of doing this could for example use the Moore Penrose pseudoinverse or some other method for system with no solution. One advantage of this might be that it always gives some value instead of doing division by zero in points where the surface is not regular, but it is probably also more expensive, because it requires an inverse of a 3x3 matrix.
			const auto tU = toOrthonormalBasis(initialPositionTangentU);
			const auto tV = toOrthonormalBasis(initialPositionTangentV);
			const auto i = toOrthonormalBasis(intersection - initialPositionPos);
			const auto inUvCoordinates = Mat2(tU, tV).inversed() * i;
			// TODO: Could allow snapping to the grid.
			tangentPlaneIntersection = TangentPlanePosition{
				.inSpace = intersection,
				.inUvCoordinates = inUvCoordinates
			};
		}
	}

	if (Input::isMouseButtonDown(MouseButton::LEFT)) {
		const auto grabDistance = 0.06f;
		// Counting all intersections so the user can grab things on the other side of the transparent surface.
		for (const auto& intersection : intersections) {
			const auto pos = surface.position(intersection.uv.x, intersection.uv.y);
			if (pos.distanceTo(initialPositionPos) < grabDistance) {
				grabbed = Grabbed::POSITION;
				break;
			}
		}
		if (grabbed == Grabbed::NONE && 
			tangentPlaneIntersection &&
			tangentPlaneIntersection->inSpace.distanceTo(initialVelocityVectorEndPosition) < grabDistance) {
			grabbed = Grabbed::VELOCITY;
		}
	}
	if (Input::isMouseButtonUp(MouseButton::LEFT)) {
		grabbed = Grabbed::NONE;
	}
	switch (grabbed) {
		using enum Grabbed;
	case NONE:
		break;
	case POSITION: {
		// Sorting by the distance to the current position so that if the user grabs the thing on the other side it stays on the other side.
		std::ranges::sort(
			intersections,
			[&initialPositionPos](const Intersection& a, const Intersection& b) {
				return a.i.position.distanceTo(initialPositionPos) < b.i.position.distanceTo(initialPositionPos);
			}
		);
		if (intersections.size() >= 1) {
			initialPositionUv = intersections[0].uv;
		}
		break;
	}

	case VELOCITY: {
		if (tangentPlaneIntersection.has_value()) {
			initialVelocityUv = tangentPlaneIntersection->inUvCoordinates.normalized();
		}
		break;
	}

	}
	{
		const auto initialPosition = surface.position(initialPositionUv.x, initialPositionUv.y);
		const auto initialVelocity =
			(surface.tangentU(initialPositionUv.x, initialPositionUv.y) * initialVelocityUv.x +
			surface.tangentV(initialPositionUv.x, initialPositionUv.y) * initialVelocityUv.y).normalized();
		renderer.sphere(initialPosition, 0.015f, Color3::RED);
		const auto radius = 0.01f;
		const auto coneRadius = radius * 2.5f;
		const auto coneLength = coneRadius * 2.0f;
		renderer.arrowStartEnd(
			initialPosition,
			initialPosition + initialVelocity * initialVelocityVectorLength,
			radius,
			coneRadius,
			coneLength,
			Color3::WHITE,
			Color3::RED
		);
	}

	{
		auto movementRhs = [&](Vec4 state, f32 _) {
		const auto symbols = surface.christoffelSymbols(state.x, state.y);
			Vec2 velocity(state.z, state.w);

			return Vec4(
				velocity.x,
				velocity.y,
				-dot(velocity, symbols.x * velocity),
				-dot(velocity, symbols.y * velocity)
			);
		};

		const auto geodesicLength = 15.0f;
		const auto steps = 200;
		const auto dl = geodesicLength / steps;

		Vec2 position = initialPositionUv;
		Vec2 velocity = Vec2::oriented(initialVelocityUv.angle());
		for (i32 i = 0; i < steps; i++) {
			const auto uTangent = surface.tangentU(position.x, position.y);
			const auto vTangent = surface.tangentV(position.x, position.y);
			const auto v = (velocity.x * uTangent + velocity.y * vTangent).length();
			velocity /= v;
			i32 n = 5;
			Vec4 state(position.x, position.y, velocity.x, velocity.y);
			for (i32 j = 0; j < n; j++) {
				state = rungeKutta4Step(movementRhs, state, 0.0f, dl / n);
			}
			const auto newPosition = Vec2(state.x, state.y);
			renderer.line(surface.position(position.x, position.y), surface.position(newPosition.x, newPosition.y), 0.01f, Color3::RED);
			const auto tangent = Vec2(state.z, state.w);
			position = newPosition;
			velocity = tangent;
		}
	}

}

i32 MainLoop::Surface::vertexCount() const {
	return positions.size();
}

i32 MainLoop::Surface::triangleCount() const {
	return indices.size() / 3;
}

void MainLoop::Surface::addVertex(Vec3 p, Vec3 n, Vec2 uv, Vec2 uvt) {
	positions.push_back(p);
	normals.push_back(n);
	uvs.push_back(uv);
	uvts.push_back(uvt);
}
