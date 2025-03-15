#include <game/Game.hpp>
#include <engine/Input/Input.hpp>
#include <imgui/imgui.h>
#include <engine/Math/Mat4.hpp>
#include <glad/glad.h>
#include <game/Constants.hpp>
#include <engine/Window.hpp>
#include <engine/Math/Constants.hpp>

Game::Game()
	: renderer(GameRenderer::make()) {

	surface.initialize(Surface::Type::TORUS);
	Window::disableCursor();
}

void Game::update() {
	/*if (Input::isKeyDown(KeyCode::F3)) {
		showGui = !showGui;
	}*/

	/*if (showGui) {
		gui();
	}*/

	if (Input::isKeyDown(KeyCode::TAB)) {
		if (cameraMode == CameraMode::ON_SURFACE) {
			cameraMode = CameraMode::IN_SPACE;
		} else if (cameraMode == CameraMode::IN_SPACE) {
			cameraMode = CameraMode::ON_SURFACE;
		}
		ImGui::SetWindowFocus(nullptr);
	}

	if (Input::isKeyDown(KeyCode::X)) {
		Window::close();
	}

	auto view = Mat4::identity;
	Vec3 cameraPosition(0.0f);
	switch (cameraMode) {
		using enum CameraMode;
	case ON_SURFACE: {
		const auto r = surfaceCamera.update(surface, Constants::dt);
		view = r;
		cameraPosition = surfaceCamera.cameraPosition(surface);
		break;
	}

	case IN_SPACE: {
		fpsCamera.update(Constants::dt);
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

	//renderer.renderCyllinders();
	//renderer.renderHemispheres();
	//renderer.renderCones();
	//renderer.renderCircles();

	const auto meshOpacity = 0.5f;
	const auto isVisible = meshOpacity > 0.0f;
	const auto isTransparent = meshOpacity < 1.0f;
	if (isTransparent) {
		surface.sortTriangles(cameraPosition);
	}

	const auto indicesOffset = i32(renderer.surfaceTriangles.currentIndex());
	for (i32 i = 0; i < surface.vertexCount(); i++) {
		const auto position = surface.positions[i];

		const auto uvt = surface.uvts[i];

		renderer.surfaceTriangles.addVertex(Vertex3Pnt{
			.position = position ,
			.normal = surface.normals[i],
			.uv = surface.uvts[i]
		});
	}
	for (i32 i = 0; i < surface.triangleCount(); i++) {
		const auto index = indicesOffset + surface.sortedTriangles[i] * 3;
		const auto i0 = surface.indices[index];
		const auto i1 = surface.indices[index + 1];
		const auto i2 = surface.indices[index + 2];
		renderer.surfaceTriangles.addTri(i0, i1, i2);
	}

	if (isTransparent) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(GL_FALSE);
	}

	renderer.renderSurfaceTriangles(meshOpacity); 

	if (isTransparent) {
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
	}
}
