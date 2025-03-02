#include "MainLoop.hpp"

MainLoop::MainLoop()
	: renderer(Renderer::make()) 
	, curveVisualization(CurveVisualization::make()) {

	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	Window::disableCursor();
	//Window::setSize(Vec2T<i32>(720, 1280) * 19 / 30);
}

void MainLoop::update() {
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

	if (Input::isKeyDown(KeyCode::X)) {
		Window::close();
	}

	switch (mode) {
		using enum Mode;
	case SURFACE: surfaceVisualization.update(renderer); break;
	case CURVE: curveVisualization.update(renderer); break;
	}
}
