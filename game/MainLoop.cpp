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

template<typename T>
void getTriangle(const std::vector<T>& values, const std::vector<i32>& indices, T* triangle, i32 triangleIndex) {
	for (i32 i = 0; i < 3; i++) {
		const auto index = indices[triangleIndex * 3 + i];
		triangle[i] = values[index];
	}
};

const auto dt = 1.0f / 60.0f;

void initializeSurface(
	const RectParametrization auto& parametrization,
	MainLoop::Surface& surface) {
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

void MainLoop::initializeParticles(
	FlowParticles& particles,
	i32 particleCount,
	const RectParametrization auto& surface,
	const VectorField auto& vectorField) {

	particles.initialize(particleCount);
	for (i32 i = 0; i < particleCount; i++) {
		randomInitializeParticle(surface, i);
	}
	for (i32 i = 0; i < particleCount; i++) {
		const auto elapsed = std::uniform_int_distribution<i32>(0, particles.lifetime[i] - 1)(rng);
		for (i32 j = 1; j <= elapsed; j++) {
			particles.position(i, j) = particles.position(i, 0);
		}
		particles.elapsed[i] = elapsed;
	}

}

void MainLoop::updateParticles(const RectParametrization auto& surface, const VectorField auto& vectorField) {
//	auto particle = [this](Vec3 v, f32 a, f32 size, Vec3 color) {
//	renderer.flowParticle(size, v, Vec4(color, a));
//};
	static bool stopped = false;
	//ImGui::Checkbox("stopped", &stopped);
	bool step = !stopped;
	/*if (ImGui::Button("step")) {
		step = true;
	}*/
	// Updating every other frame makes it look laggy.
	// creation frame = 0
	// normally updates for lifetime frames. On frame = lifetime - 1 is the last update.
	// When frame >= lifetime then the it starts disappearing. 
	// On frame lifetime + disappearTime it would fully disappear so instead it's respawned.
	const auto disappearTime = 10;
	for (i32 i = 0; i < flowParticles.particleCount(); i++) {
		const auto& lifetime = flowParticles.lifetime[i];
		auto& elapsed = flowParticles.elapsed[i];
		if (step) {
			elapsed++;
			const auto disapperElapsed = std::max(0, elapsed - lifetime);
			if (elapsed < lifetime) {
				const auto p = flowParticles.position(i, elapsed - 1);
				const auto velocity = flowParticles.velocity(i, elapsed - 1);
				const auto newPosition = p + velocity * dt * 3.0f;
				flowParticles.position(i, elapsed) = newPosition;
				flowParticles.normal(i, elapsed) = surface.normal(newPosition.x, newPosition.y);
				{
					const auto newPosition3 = surface.position(newPosition.x, newPosition.y);
					const auto tangentU = surface.tangentU(newPosition.x, newPosition.y);
					const auto tangentV = surface.tangentV(newPosition.x, newPosition.y);

					const auto normal = flowParticles.normal(i, elapsed - 1);
					const auto vector = vectorFieldSample(newPosition3);
					auto vectorUv = vectorInTangentSpaceBasis(vector, tangentU, tangentV, normal);
					const auto l = (vectorUv.x * tangentU + vectorUv.y * tangentV).length();
					const auto color = Color3::scientificColoring(l, vectorFieldMinLength, vectorFieldMaxLength);
					flowParticles.color(i, elapsed) = color;
					flowParticles.velocity(i, elapsed) = vectorUv;
				}
			} else if (disapperElapsed >= disappearTime - 1) {
				randomInitializeParticle(surface, i);
			}
		}
		const auto disapperElapsed = std::max(0, elapsed - lifetime);


		const auto frameCount = elapsed + 1;
		for (i32 positionI = 0; positionI < std::min(frameCount, lifetime); positionI++) {
			const auto disappearT = f32(disapperElapsed) / f32(disappearTime);
			const auto p = flowParticles.position(i, positionI);
			f32 a = 0.5f;
			f32 t = 1.0f;
			t *= f32(positionI + 1) / f32(frameCount);
			t *= 1.0f - disappearT;
			const auto maxSize = 0.03f;
			const auto size = (t + 1.0f) / 2.0f * maxSize;
			auto position = surface.position(p.x, p.y);
			const auto normal = flowParticles.normal(i, positionI);
			const auto color = flowParticles.color(i, positionI);
			position += flowParticles.normal(i, positionI) * maxSize;

			renderer.flowParticle(size, position, Vec4(color, a * t));
		}

	}

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	renderer.renderFlowParticles(Mat4(fpsCamera.cameraForwardRotation().inverseIfNormalized().toMatrix()));
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
}

void sliderF32(const char* label, f32& value, f32 min, f32 max) {
	if (ImGui::SliderFloat(label, &value, min, max)) {
		value = std::clamp(value, min, max);
	}
}

MainLoop::MainLoop()
	: renderer(Renderer::make())
	, noise(5) {

	initializeSelectedSurface();
	initializeParticles(5000);

	randomizeVectorField(5);

	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	Window::disableCursor();
}

void MainLoop::update() {
	ShaderManager::update();

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
	const auto cameraForward = (Vec4(Vec3::FORWARD, 0.0f) * view.inversed()).xyz();

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

	auto flowToolUpdate = [this] {
		#define I(name, vectorField) updateParticles(surfaces.name, vectorField)
		#define S(vectorField) \
		switch (surfaces.selected) { \
			using enum Surfaces::Type; \
		case TORUS: I(torus, vectorField); break; \
		case TREFOIL: I(trefoil, vectorField); break; \
		case HELICOID: I(helicoid, vectorField); break; \
		case MOBIUS_STRIP: I(mobiusStrip, vectorField); break; \
		case PSEUDOSPHERE: I(pseudosphere, vectorField); break; \
		case CONE: I(cone, vectorField); break; \
		case SPHERE: I(sphere, vectorField); break; \
		}
		switch (selectedVectorField) {
			using enum VectorFieldType;
		case RANDOM: S([this](Vec3 pos) { return vectorFieldSample(pos); });
		}
		#undef I
		#undef S
	};

	auto spectralSample = [](f32 v) {
		// https://github.com/matplotlib/matplotlib/blob/main/lib/matplotlib/_cm.py
		// _Spectral_data 
		const Vec3 colors[] = {
			Vec3(0.61960784313725492f, 0.003921568627450980f, 0.25882352941176473f),
			Vec3(0.83529411764705885f, 0.24313725490196078f, 0.30980392156862746f),
			Vec3(0.95686274509803926f, 0.42745098039215684f, 0.2627450980392157f),
			Vec3(0.99215686274509807f, 0.68235294117647061f, 0.38039215686274508f),
			Vec3(0.99607843137254903f, 0.8784313725490196f, 0.54509803921568623f),
			Vec3(1.0f, 1.0f, 0.74901960784313726f),
			Vec3(0.90196078431372551f, 0.96078431372549022f, 0.59607843137254901f),
			Vec3(0.6705882352941176f, 0.8666666666666667f, 0.64313725490196083f),
			Vec3(0.4f, 0.76078431372549016f, 0.6470588235294118f),
			Vec3(0.19607843137254902f, 0.53333333333333333f, 0.74117647058823533f),
			Vec3(0.36862745098039218f, 0.30980392156862746f, 0.63529411764705879f)
		};
		const auto colorCount = std::size(colors);
		const auto indexFloat = v * colorCount;
		const auto index = i32(std::floor(indexFloat));
		if (index <= 0) {
			return colors[0];
		}
		if (index >= colorCount - 1) {
			return colors[colorCount - 1];
		}
		const auto t = indexFloat - index;
		return lerp(colors[index], colors[index + 1], t);
	};

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
				Vec3 color = spectralSample(1.0f - t);
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
		flowToolUpdate();
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
		randomizeVectorField(rng());
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
			initializeParticles(5000); \
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
		case FLOW: return "flow";
		}
		return "";
	};
	ToolInfo tools[]{
		{ ToolType::NONE },
		{ ToolType::GEODESICS },
		{ ToolType::FLOW },
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

void MainLoop::initializeParticles(i32 particleCount) {
	#define I(surface, vectorField) initializeParticles(flowParticles, particleCount, surface, vectorField)
	#define S(vectorField) \
	switch (surfaces.selected) { \
		using enum Surfaces::Type; \
	case TORUS: I(surfaces.torus, vectorField); break; \
	case TREFOIL: I(surfaces.trefoil, vectorField); break; \
	case HELICOID: I(surfaces.helicoid, vectorField); break; \
	case MOBIUS_STRIP: I(surfaces.mobiusStrip, vectorField); break; \
	case PSEUDOSPHERE: I(surfaces.pseudosphere, vectorField); break; \
	case CONE: I(surfaces.cone, vectorField); break; \
	case SPHERE: I(surfaces.sphere, vectorField); break; \
	}
	switch (selectedVectorField) {
		using enum VectorFieldType;
	case RANDOM: S([this](Vec3 pos) { return vectorFieldSample(pos); });
	}
	#undef I
	#undef S
}

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
	}
	#undef U
}

Vec2 MainLoop::randomPointOnSurface() {
	const auto value = uniform01(rng) * surfaceData.totalArea;
	f32 cursor = 0.0f;
	i32 randomTriangleIndex = 0;
	for (i32 i = 0; i < surfaceData.triangleCount(); i++) {
		cursor += surfaceData.triangleAreas[i];
		if (cursor >= value) {
			randomTriangleIndex = i;
			break;
		}
	}
	Vec2 uvs[3];
	getTriangle(surfaceData.uvs, surfaceData.indices, uvs, randomTriangleIndex);
	const auto r0 = uniform01(rng);
	const auto r1 = uniform01(rng);
	return uniformRandomPointOnTri(uvs, r0, r1);
}

void MainLoop::randomInitializeParticle(const RectParametrization auto& surface, i32 i) {
	const auto p = randomPointOnSurface();
	const auto lifetime = std::uniform_int_distribution<i32>(
		i32(FlowParticles::maxLifetime * f32(0.7f)),
		FlowParticles::maxLifetime)(rng);

	const auto position = surface.position(p.x, p.y);
	const auto tangentU = surface.tangentU(p.x, p.y);
	const auto tangentV = surface.tangentV(p.x, p.y);
	const auto normal = cross(tangentU, tangentV).normalized();
	const auto vector = vectorFieldSample(position);
	auto vectorUv = vectorInTangentSpaceBasis(vector, tangentU, tangentV, normal);
	const auto l = (vectorUv.x * tangentU + vectorUv.y * tangentV).length();
	const auto color = Color3::scientificColoring(l, vectorFieldMinLength, vectorFieldMaxLength);

	//const auto lifetime = FlowParticles::maxLifetime;
	flowParticles.initializeParticle(i, p, normal, lifetime, color, vectorUv);
}

void MainLoop::randomizeVectorField(usize seed) {
	noise = PerlinNoise(seed);
	vectorFieldMinLength = std::numeric_limits<f32>::infinity();
	vectorFieldMaxLength = -std::numeric_limits<f32>::infinity();
	for (i32 i = 0; i < surfaceData.triangleCount(); i++) {
		Vec3 v[3];
		getTriangle(surfaceData.positions, surfaceData.indices, v, i);
		const auto normal = cross(v[1] - v[0], v[2] - v[0]).normalized();
		const auto centroid = (v[0] + v[1] + v[2]) / 3.0f;
		const auto vector = vectorFieldSample(centroid);
		const auto projectedOntoTangentSpace = vector - dot(vector, normal) * normal;
		const auto length = projectedOntoTangentSpace.lengthSquared();
		if (length > vectorFieldMaxLength) {
			vectorFieldMaxLength = length;
		}
		if (length < vectorFieldMinLength) {
			vectorFieldMinLength = length;
		}
	}
	vectorFieldMaxLength = sqrt(vectorFieldMaxLength);
	vectorFieldMinLength = sqrt(vectorFieldMinLength);
}

Vec3 MainLoop::vectorFieldSample(Vec3 v) const {
	v /= 10.0f;
	return Vec3(
		noise.value3d(v),
		noise.value3d(v + Vec3(214.0f, 0.0f, 0.0f)),
		noise.value3d(v + Vec3(0.0f, 24.456f, 0.0f))
	);
}

void MainLoop::Surface::sortTriangles(Vec3 cameraPosition) {
	// @Performance: Could discard triangles behind the camera.
	std::vector<f32> distances;
	for (i32 i = 0; i < triangleCenters.size(); i++) {
		distances.push_back(triangleCenters[i].distanceSquaredTo(cameraPosition));
	}
	const auto lessThan = [&](i32 a, i32 b) {
		return distances[a] > distances[b];
	};
	std::sort(sortedTriangles.begin(), sortedTriangles.end(), lessThan);
}

i32 MainLoop::Surface::vertexCount() const {
	return i32(positions.size());
}

i32 MainLoop::Surface::triangleCount() const {
	return i32(indices.size() / 3);
}

void MainLoop::Surface::addVertex(Vec3 p, Vec3 n, Vec2 uv, Vec2 uvt) {
	positions.push_back(p);
	normals.push_back(n);
	uvs.push_back(uv);
	uvts.push_back(uvt);
}

Vec2& FlowParticles::position(i32 particleIndex, i32 frame) {
	ASSERT(particleIndex < particleCount());
	if (lifetime[particleIndex] > 0) {
		ASSERT(frame < lifetime[particleIndex]);
	}
	return positionsData[maxLifetime * particleIndex + frame];
}

Vec3& FlowParticles::normal(i32 particleIndex, i32 frame) {
	return normalsData[maxLifetime * particleIndex + frame];
}

Vec3& FlowParticles::color(i32 particleIndex, i32 frame) {
	return colorsData[maxLifetime * particleIndex + frame];
}

Vec2& FlowParticles::velocity(i32 particleIndex, i32 frame) {
	return velocitiesData[maxLifetime * particleIndex + frame];
}

void FlowParticles::initialize(i32 particleCount) {
	positionsData.resize(particleCount * maxLifetime);
	normalsData.resize(particleCount * maxLifetime);
	velocitiesData.resize(particleCount * maxLifetime);
	colorsData.resize(particleCount * maxLifetime);
	lifetime.resize(particleCount);
	elapsed.resize(particleCount);
}

i32 FlowParticles::particleCount() const {
	return i32(lifetime.size());
}

void FlowParticles::initializeParticle(i32 i, Vec2 position, Vec3 normal, i32 lifetime, Vec3 color, Vec2 velocity) {
	this->position(i, 0) = position;
	this->normal(i, 0) = normal;
	this->velocity(i, 0) = velocity;
	this->color(i, 0) = color;
	this->lifetime[i] = lifetime;
	this->elapsed[i] = 0;
}

