#include "MainLoop.hpp"
#include <glad/glad.h>
#include <engine/Math/Color.hpp>
#include <engine/Math/OdeIntegration/RungeKutta4.hpp>
#include <engine/Math/Interpolation.hpp>
#include <game/PlotUtils.hpp>
#include <engine/Window.hpp>
#include <engine/Input/Input.hpp>
#include <imgui/implot.h>
#include <gfx/ShaderManager.hpp>
#include <game/Tri3d.hpp>
#include <game/Utils.hpp>
#include <random>
#include <game/MeshUtils.hpp>

const auto dt = 1.0f / 60.0f;

void initializeSurface(
	const RectParametrization auto& parametrization,
	SurfaceData& surface) {
	//const auto size = 100;
	const auto size = 50;
	surface.indices.clear();
	surface.positions.clear();
	surface.uvs.clear();
	surface.uvts.clear();
	surface.normals.clear();
	surface.curvatures.clear();

	for (i32 vi = 0; vi <= size; vi++) {
		for (i32 ui = 0; ui <= size; ui++) {
			const auto ut = f32(ui) / size;
			const auto vt = f32(vi) / size;
			const auto u = lerp(parametrization.uMin, parametrization.uMax, ut);
			const auto v = lerp(parametrization.vMin, parametrization.vMax, vt);
			const auto p = parametrization.position(u, v);
			const auto n = parametrization.normal(u, v);

			surface.curvatures.push_back(parametrization.curvature(u, v));
			surface.addVertex(p, n, Vec2(u, v), Vec2(ut, vt));
		}
	}
	{
		const auto r = std::ranges::minmax(surface.curvatures);
		surface.minCurvature = r.min;
		surface.maxCurvature = r.max;
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

	surface.triangleCenters.clear();
	for (i32 i = 0; i < surface.triangleCount(); i++) {
		Vec3 triangle[3];
		getTriangle(surface.positions, surface.indices, triangle, i);
		surface.triangleCenters.push_back(triCenter(triangle));
	}

	surface.sortedTriangles.clear();
	for (i32 i = 0; i < surface.triangleCount(); i++) {
		surface.sortedTriangles.push_back(i);
	}

	surface.triangleAreas.clear();
	for (i32 i = 0; i < surface.triangleCount(); i++) {
		Vec3 vs[3];
		getTriangle(surface.positions, surface.indices, vs, i);
		surface.triangleAreas.push_back(triArea(vs));
	}
	f32 totalArea = 0.0f;
	for (i32 i = 0; i < surface.triangleCount(); i++) {
		totalArea += surface.triangleAreas[i];
	}
	surface.totalArea = totalArea;
}

void sliderF32(const char* label, f32& value, f32 min, f32 max) {
	if (ImGui::SliderFloat(label, &value, min, max)) {
		value = std::clamp(value, min, max);
	}
}

MainLoop::MainLoop()
	: renderer(Renderer::make()) {

	initializeSelectedSurface();
	vectorFieldTool.randomizeVectorField(surfaceData, surfaces);
	vectorFieldTool.initializeParticles(surfaces, surfaceData, 5000);

	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	Window::disableCursor();
}

void MainLoop::update() {
	ShaderManager::update();

	//vis.update(renderer);

	//return;

	if (Input::isKeyDown(KeyCode::ESCAPE)) {
		Window::toggleCursor();
	}

	auto id = ImGui::DockSpaceOverViewport(
		ImGui::GetMainViewport(),
		ImGuiDockNodeFlags_NoDockingOverCentralNode | ImGuiDockNodeFlags_PassthruCentralNode);

	{
		const auto cursorEnabled = Window::isCursorEnabled();

		const auto flags =
			ImGuiConfigFlags_NavNoCaptureKeyboard |
			ImGuiConfigFlags_NoMouse |
			ImGuiConfigFlags_NoMouseCursorChange;

		if (cursorEnabled) {
			ImGui::GetIO().ConfigFlags &= ~flags;
		} else {
			ImGui::GetIO().ConfigFlags |= flags;
		}
	}

	/*Input::ignoreImGuiWantCapture = Window::isCursorEnabled();*/

	if (Input::isKeyDown(KeyCode::X)) {
		Window::close();
	}

	if (Input::isKeyDown(KeyCode::F3)) {
		showGui = !showGui;
	}

	if (showGui) {
		gui();
	}

	if (Input::isKeyDown(KeyCode::TAB)) {
		if (cameraMode == CameraMode::ON_SURFACE) {
			cameraMode = CameraMode::IN_SPACE;
		} else if (cameraMode == CameraMode::IN_SPACE) {
			cameraMode = CameraMode::ON_SURFACE;
		}
		ImGui::SetWindowFocus(nullptr);
	}

	auto view = Mat4::identity;
	Vec3 cameraPosition(0.0f);
	switch (cameraMode) {
		using enum CameraMode;
	case ON_SURFACE: {
		const auto r = updateSurfaceCamera(dt);
		view = r.view;
		cameraPosition = r.cameraPosition;
		break;
	}

	case IN_SPACE: {
		fpsCamera.update(dt);
		view = fpsCamera.viewMatrix();
		cameraPosition = fpsCamera.position;
		break;
	}
		
	}
	// Could convert to Mat3 and just do a transpose.
	const auto cameraForward = (Vec4(Vec3::FORWARD, 0.0f) * view.inversed()).xyz().normalized();

  	const auto aspectRatio = Window::aspectRatio();
	const auto projection = Mat4::perspective(PI<f32> / 2.0f, aspectRatio, 0.1f, 1000.0f);
	const auto swaxpYZ = Mat4(Mat3(Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f), Vec3(0.0f, 1.0f, 0.0f)));
	renderer.transform = projection * view;
	renderer.view = view;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, i32(Window::size().x), i32(Window::size().y));

	renderer.renderCyllinders();
	renderer.renderHemispheres();
	renderer.renderCones();
	renderer.renderCircles();

	const auto isVisible = meshOpacity > 0.0f;
	if (isVisible && selectedTool != ToolType::FLOW) {
		const auto isTransparent = meshOpacity < 1.0f;
		if (isTransparent) {
			surfaceData.sortTriangles(cameraPosition);
		}

		const auto indicesOffset = i32(renderer.coloredTriangles.currentIndex());
		for (i32 i = 0; i < surfaceData.vertexCount(); i++) {
			const auto position = surfaceData.positions[i];

			const auto uvt = surfaceData.uvts[i];
			//const auto initialPosition = Vec3(uvt.x, 0.0f, uvt.y) - Vec3(0.5f, 0.5f, 0.5f);
			const auto initialPosition = Vec3(-uvt.x, 0.0f, -uvt.y) + Vec3(0.5f, 0.5f, 0.5f);
			const auto p = lerp(initialPosition, position, transitionT);

			switch (meshRenderMode) {
				using enum MeshRenderMode;
			case GRID: {
				renderer.triangles.addVertex(Vertex3Pnt{
					.position = p,
					.normal = surfaceData.normals[i],
					.uv = surfaceData.uvts[i]
				});
				break;
			}
			case CURVATURE: {
				const auto biggest = std::max(
					std::abs(surfaceData.minCurvature), 
					std::abs(surfaceData.maxCurvature));
				auto t = surfaceData.curvatures[i];
				t /= biggest;
				t += 1.0f;
				t /= 2.0f;
				Vec3 color = Color3::spectral(1.0f - t);
				renderer.coloredTriangles.addVertex(Vertex3Pnc{
					.position = p,
					.normal = surfaceData.normals[i],
					.color = color
				});
				break;
			}

			}
		}
		for (i32 i = 0; i < surfaceData.triangleCount(); i++) {
			const auto index = indicesOffset + surfaceData.sortedTriangles[i] * 3;
			const auto i0 = surfaceData.indices[index];
			const auto i1 = surfaceData.indices[index + 1];
			const auto i2 = surfaceData.indices[index + 2];
			switch (meshRenderMode) {
				using enum MeshRenderMode;
			case GRID: 
				renderer.triangles.addTri(i0, i1, i2);
				break;

			case CURVATURE: 
				renderer.coloredTriangles.addTri(i0, i1, i2);
				break;
			}
			
		}

		if (isTransparent) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDepthMask(GL_FALSE);
		}
		switch (meshRenderMode) {
			using enum MeshRenderMode;
		case GRID: renderer.renderTriangles(meshOpacity); break;
		case CURVATURE: renderer.renderColoredTriangles(meshOpacity); break;
		}
		if (isTransparent) {
			glDisable(GL_BLEND);
			glDepthMask(GL_TRUE);
		}
	}
	

	calculateIntersections(cameraPosition, cameraForward);

	switch (selectedTool) {
	using enum ToolType;
	case NONE: {
		break;
	}
	case GEODESICS: {
		geodesicTool.update(cameraPosition, cameraForward, intersections, surfaces, renderer);
		break;
	}

	case FLOW: {
		vectorFieldTool.update(view.inversed(), cameraPosition, cameraForward, renderer, surfaces, surfaceData);
		break;
	}

	case CURVATURE: {
		curvatureTool.update(intersections, surfaces, renderer);
		break;
	}

	}
	//renderer.sphere(cameraPosition + cameraForward, 0.02f, Color3::GREEN);

	if (showGui) {
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		renderer.gfx2d.disk(Vec2(0.0f), 0.015f, Color3::WHITE);
		renderer.gfx2d.drawDisks();
		glDisable(GL_BLEND);
	}
}

void MainLoop::gui() {

	ImGui::Begin("settings");
	if (ImGui::Button("randomize")) {
		vectorFieldTool.randomizeVectorField(surfaceData, surfaces);
	}

	CameraMode cameraModes[]{ CameraMode::IN_SPACE, CameraMode::ON_SURFACE };
	auto cameraModeStr = [](CameraMode mode) -> const char* {
		switch (mode) {
			using enum CameraMode;
		case IN_SPACE: return "in space";
		case ON_SURFACE: return "on surface";
		}
		return "";
	};

	if (ImGui::BeginCombo("camera mode", cameraModeStr(cameraMode))) {
		for (auto& mode : cameraModes) {
			const auto isSelected = mode == cameraMode;
			if (ImGui::Selectable(cameraModeStr(mode), isSelected)) {
				cameraMode = mode;
			}
			if (isSelected) { ImGui::SetItemDefaultFocus(); }
		}
		ImGui::EndCombo();
	}

	switch (cameraMode) {
		using enum CameraMode;
	case ON_SURFACE: {
		if (ImGui::Button("walk on other side")) {
			surfaceCamera.normalFlipped = !surfaceCamera.normalFlipped;
		}
		break;
	}
	case IN_SPACE:
		break;
	}
	ImGui::Separator();

	#define S(ENUM_NAME, name) {\
		const auto isSelected = surfaces.selected == ENUM_NAME; \
		if (ImGui::Selectable(surfaceNameStr(ENUM_NAME), isSelected)) { \
			surfaces.selected = ENUM_NAME; \
			initializeSelectedSurface(); \
			vectorFieldTool.initializeParticles(surfaces, surfaceData, 5000); \
		} \
		if (isSelected) { ImGui::SetItemDefaultFocus(); } \
	}

	const auto selectedName = surfaceNameStr(surfaces.selected);
	if (ImGui::BeginCombo("surface", selectedName)) {
		using enum Surfaces::Type;
		S(TORUS, torus);
		S(TREFOIL, trefoil);
		S(HELICOID, helicoid);
		S(MOBIUS_STRIP, mobiusStrip);
		S(PSEUDOSPHERE, pseudosphere);
		S(CONE, cone);
		S(SPHERE, sphere);
		S(PROJECTIVE_PLANE, projectivePlane);
		ImGui::EndCombo();
	}
	#undef S

	auto meshRenderModeName = [](MeshRenderMode mode) -> const char* {
		switch (mode) {
			using enum MeshRenderMode;
		case GRID: return "grid";
		case CURVATURE: return "gaussian curvature";
		}
		return "";
	};
	MeshRenderMode renderModes[]{
		MeshRenderMode::GRID,
		MeshRenderMode::CURVATURE,
	};
	if (ImGui::BeginCombo("mesh coloring", meshRenderModeName(meshRenderMode))) {
		for (auto& mode : renderModes) {
			const auto isSelected = mode == meshRenderMode;
			if (ImGui::Selectable(meshRenderModeName(mode), isSelected)) {
				meshRenderMode = mode;
			}
			if (isSelected) { ImGui::SetItemDefaultFocus(); }
		}
		ImGui::EndCombo();
	}

	sliderF32("opacity", meshOpacity, 0.0f, 1.0f);
	sliderF32("transition", transitionT, 0.0f, 1.0f);

	struct ToolInfo {
		ToolType type;
		//const char* tooltip;
	};
	auto toolName = [](ToolType tool) -> const char* {
		switch (tool) {
			using enum ToolType;
		case NONE: return "none";
		case GEODESICS: return "geodesics";
		case FLOW: return "vector field";
		case CURVATURE: return "curvature";
		}
		return "";
	};
	ToolInfo tools[]{
		{ ToolType::NONE },
		{ ToolType::GEODESICS },
		{ ToolType::FLOW },
		{ ToolType::CURVATURE },
	};
	ImGui::Separator();
	if (ImGui::BeginCombo("tool", toolName(selectedTool))) {
		for (auto& tool : tools) {
			const auto isSelected = tool.type == selectedTool;
			if (ImGui::Selectable(toolName(tool.type), isSelected)) {
				selectedTool = tool.type;
			}
			if (isSelected) { ImGui::SetItemDefaultFocus(); }
		}
		ImGui::EndCombo();
	}
	switch (selectedTool) {
		using enum ToolType;
	case NONE:
		break;
	case GEODESICS:
		break;
	case CURVATURE:
		break;
	case FLOW: {
		ImGui::Checkbox("show flow", &vectorFieldTool.showFlow);
		ImGui::Checkbox("show vectors", &vectorFieldTool.showVectors);
		{
			using enum VectorFieldTool::VectorFieldType;
			auto vectorFieldTypeName = [](VectorFieldTool::VectorFieldType& type) -> const char* {
				switch (type) {
				case CUSTOM: return "custom";
				case RANDOM: return "random";
				}
			};

			VectorFieldTool::VectorFieldType fieldTypes[]{
				RANDOM,
				CUSTOM
			};
			if (ImGui::BeginCombo("vector field", vectorFieldTypeName(vectorFieldTool.selectedVectorField))) {
				for (auto& field : fieldTypes) {
					const auto isSelected = field == vectorFieldTool.selectedVectorField;
					if (ImGui::Selectable(vectorFieldTypeName(field), isSelected)) {
						vectorFieldTool.selectedVectorField = field;
						vectorFieldTool.initializeValues(surfaces, surfaceData);
						vectorFieldTool.initializeParticles(surfaces, surfaceData, 5000);
					}
					if (isSelected) { ImGui::SetItemDefaultFocus(); }
				}
				ImGui::EndCombo();
			}
		}
		
		break;
	}
		
	}
	ImGui::End();
}

void MainLoop::initializeSelectedSurface() {
	#define I(name) initializeSurface(surfaces.name, surfaceData); break;
	switch (surfaces.selected) {
		using enum Surfaces::Type;
	case TORUS: I(torus);
	case TREFOIL: I(trefoil);
	case HELICOID: I(helicoid);
	case MOBIUS_STRIP: I(mobiusStrip);
	case PSEUDOSPHERE: I(pseudosphere);
	case CONE: I(cone);
	case SPHERE: I(sphere);
	case PROJECTIVE_PLANE: I(projectivePlane);
	}
	#undef I
}

//#define SWITCH_ON_SURFACE(surfaceType, F, ...) \
//switch (surfaceType) { \
//	case TORUS: F(torus, __VA_ARGS__); break; \
//	case TREFOIL: F(trefoil, __VA_ARGS__); break; \
//	case HELICOID: F(helicoid, __VA_ARGS__); break; \
//	case MOBIUS_STRIP: F(mobiusStrip, __VA_ARGS__); break; \
//	case PSEUDOSPHERE: F(pseudosphere, __VA_ARGS__); break; \
//	case CONE: F(cone, __VA_ARGS__); break; \
//	case SPHERE: F(sphere, __VA_ARGS__); break; \
//}
// If the order doesn't match use another macro.

void MainLoop::calculateIntersections(Vec3 cameraPosition, Vec3 cameraForward) {
	intersections.clear();

	//std::vector<MeshIntersection> intersections;
	for (i32 i = 0; i < surfaceData.triangleCount(); i++) {
		Vec3 vs[3];
		getTriangle(surfaceData.positions, surfaceData.indices, vs, i);
		const auto intersection = rayTriIntersection(cameraPosition, cameraForward, vs);
		if (!intersection.has_value()) {
			continue;
		}
		Vec2 uvs[3];
		getTriangle(surfaceData.uvs, surfaceData.indices, uvs, i);
		const auto uv = barycentricInterpolate(intersection->barycentricCoordinates, uvs);
		intersections.push_back({ *intersection, i, uv, intersection->position });
	}
}

MainLoop::SurfaceCameraUpdateResult MainLoop::updateSurfaceCamera(f32 dt) {
	#define U(surface) { \
		const auto view = surfaceCamera.update(surfaces.surface, dt); \
		return SurfaceCameraUpdateResult{ view, surfaceCamera.cameraPosition(surfaces.surface) }; \
	}
	switch (surfaces.selected) {
		using enum Surfaces::Type;
	case TORUS: U(torus);
	case TREFOIL: U(trefoil);
	case HELICOID: U(helicoid);
	case MOBIUS_STRIP: U(mobiusStrip);
	case PSEUDOSPHERE: U(pseudosphere);
	case CONE: U(cone);
	case SPHERE: U(sphere);
	case PROJECTIVE_PLANE: U(projectivePlane);
	}
	#undef U
}