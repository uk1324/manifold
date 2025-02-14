#include "MainLoop.hpp"
#include <glad/glad.h>
#include <engine/Math/Color.hpp>
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

const auto dt = 1.0f / 60.0f;

void generateParametrizationOfRectangle(
	MainLoop::Surface& surface,
	auto position,
	auto normal,
	f32 uMin, f32 uMax, f32 vMin, f32 vMax) {
		{
			const auto size = 100;
			for (i32 vi = 0; vi <= size; vi++) {
				for (i32 ui = 0; ui <= size; ui++) {
					const auto ut = f32(ui) / size;
					const auto vt = f32(vi) / size;
					const auto u = lerp(uMin, uMax, ut);
					const auto v = lerp(vMin, vMax, vt);
					const auto p = position(u, v);
					const auto n = normal(u, v);
					surface.addVertex(p, n, Vec2(ut, vt));
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
		}
}

//Torus surface{ .r = 0.4f, .R = 2.0f };
Trefoil surface{ .r = 0.4f, .R = 2.0f };
/*Helicoid surface{ .uMin = -PI<f32>, .uMax = PI<f32>, .vMin = -5.0f, .vMax = 5.0f };*/
//MobiusStrip surface;
//Pseudosphere surface{ .r = 2.0f };
//Cone surface{
//	.a = 1.0f,
//	.b = 1.0f,
//	.uMin = -2.0f,
//	.uMax = 2.0f,
//};
//Sphere surface{ .r = 1.0f, };

MainLoop::MainLoop()
	: renderer(Renderer::make()) {

	generateParametrizationOfRectangle(
		surfaceMesh,
		[&](f32 u, f32 v) { return surface.position(u, v); },
		[&](f32 u, f32 v) { return surface.normal(u, v); },
		surface.uMin, surface.uMax,
		surface.vMin, surface.vMax
	);

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
	switch (cameraMode) {
		using enum CameraMode;
	case ON_SURFACE: {
		view = surfaceCamera.update(surface, dt);
		break;
	}

	case IN_SPACE: {
		fpsCamera.update(dt);
		view = fpsCamera.viewMatrix();
		break;
	}
		
	}
	const auto cameraPosition = -view.getTranslation();

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
			.uv = surfaceMesh.uvs[i] 
		});
	}

	std::vector<i32> triangles;
	for (i32 i = 0; i < surfaceMesh.triangleCount(); i++) {
		triangles.push_back(i);
	}
	std::sort(
		triangles.begin(),
		triangles.end(),
		[&](i32 a, i32 b) {
			auto getTriangle = [&](Vec3* triangle, i32 triangleStartIndexIndex) {
				for (i32 i = 0; i < 3; i++) {
					const auto index = surfaceMesh.indices[triangleStartIndexIndex];
					triangle[i] = surfaceMesh.positions[index];
				}
			};
			Vec3 av[3];
			getTriangle(av, a * 3);
			Vec3 bv[3];
			getTriangle(bv, b * 3);
			const auto ac = triCenter(av);
			const auto bc = triCenter(bv);
			return cameraPosition.distanceSquaredTo(ac) > cameraPosition.distanceSquaredTo(bc);
		}
	);

	for (i32 i = 0; i < surfaceMesh.triangleCount(); i++) {
		const auto index = indicesOffset + triangles[i] * 3;
		/*const auto index = indicesOffset + i * 3;*/
		renderer.addTriangle(
			surfaceMesh.indices[index],
			surfaceMesh.indices[index + 1],
			surfaceMesh.indices[index + 2]
		);
	}

	renderer.renderCyllinders();
	renderer.renderHemispheres();

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

	uvPositions.push_back(surfaceCamera.uvPosition);
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

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	renderer.renderTriangles(0.5f);
	glDisable(GL_BLEND);
}

i32 MainLoop::Surface::vertexCount() const {
	return positions.size();
}

i32 MainLoop::Surface::triangleCount() const {
	return indices.size() / 3;
}

void MainLoop::Surface::addVertex(Vec3 p, Vec3 n, Vec2 uv) {
	positions.push_back(p);
	normals.push_back(n);
	uvs.push_back(uv);
}
