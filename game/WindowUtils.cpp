#include "WindowUtils.hpp"
#include <engine/Input/Input.hpp>
#include <engine/Window.hpp>
#include <imgui/imgui.h>

void togglableCursorUpdate() {
	if (Input::isKeyDown(KeyCode::ESCAPE)) {
		Window::toggleCursor();
	}
	const auto cursorEnabled = Window::isCursorEnabled();
	const auto flags =
		ImGuiConfigFlags_NavNoCaptureKeyboard |
		ImGuiConfigFlags_NoMouse |
		ImGuiConfigFlags_NoMouseCursorChange;

	if (cursorEnabled) {
		Input::ignoreImGuiWantCapture = false;
		ImGui::GetIO().ConfigFlags &= ~flags;
	} else {
		Input::ignoreImGuiWantCapture = true;
		ImGui::GetIO().ConfigFlags |= flags;
	}
}