#include <engine/Engine.hpp>
#include <engine/EngineUpdateLoop.hpp>
#include <game/MainLoop.hpp>
#include <Timer.hpp>

#ifdef FINAL_RELEASE
#define FONT "assets/fonts/RobotoMono-Regular.ttf"
#else 
#define FONT "engine/assets/fonts/RobotoMono-Regular.ttf"
#endif

#include <imgui/imgui.h>

#ifdef __EMSCRIPTEN__

#include <GLFW/emscripten_glfw3.h>
#include <emscripten/html5.h>

EngineUpdateLoop updateLoop(60.0f);
MainLoop* mainLoop = nullptr;
FixedUpdateLoop l(60.0f);

#include <iostream>
#include <engine/Input/Input.hpp>
#include <opengl/gl.h>
void loop() {
	if (l.isRunning()) {
		// The order in which EngineUpdateLoop doesn't work, because of when loop is called and what happens before and after the call to it.
		Engine::updateFrameStart();
		//if (Input::isKeyDown(KeyCode::A)) {
		//	std::cout << "test\n";
		//}
		/*glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ImGui::ShowDemoWindow();
		Engine::updateFrameEnd();*/
		if (mainLoop != nullptr) {
			mainLoop->update();
		}
		Engine::updateFrameEnd();
	} else {
		Engine::terminateAll();
		emscripten_cancel_main_loop();
	}

	//if (updateLoop.isRunning()) {
	//	//if (Input::isKeyDown(KeyCode::A)) {
	//	//	std::cout << "test\n";
	//	//}
	//	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//	ImGui::ShowDemoWindow();
	//	/*if (mainLoop != nullptr) {
	//		mainLoop->update();
	//	}*/
	//} else {
	//	Engine::terminateAll();
	//	emscripten_cancel_main_loop();
	//}
}

#include "generated/FontTtf.cpp"

//const unsigned char binary_data[] = {
//	#embed "game/Animation.hpp"
//};

int main() {
	Engine::initAll(Window::Settings{
		.maximized = true,
		.multisamplingSamplesPerPixel = 16
	}, nullptr);

	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontFromMemoryCompressedTTF(font_compressed_data, font_compressed_size);

	{
		Timer t;
		mainLoop = new MainLoop();
		t.tookSeconds("initializing MainLoop");
	}

	emscripten_set_click_callback(
		"#canvas", nullptr, false, 
		+[](int eventType, const EmscriptenMouseEvent* mouseEvent, void* userData) -> bool {
			emscripten_request_pointerlock("#canvas", false);
			return true;
		}
	);

	emscripten_set_keydown_callback(
		EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr, 0, 
		+[](int eventType, const EmscriptenKeyboardEvent* e, void* userData) -> bool {
			if (e->keyCode == 'F') {
				emscripten_request_pointerlock("#canvas", false);
				emscripten_request_fullscreen("#canvas", false);
				//std::cout << "Key down: " << e->keyCode << "\n";
			}
			return true;
		}
	);

	emscripten::glfw3::AddBrowserKeyCallback([](GLFWwindow* window, int key, int scancode, int action, int mods) {
		return mods == 0 && action == GLFW_PRESS && key == GLFW_KEY_F12;
	});

	emscripten_set_main_loop(loop, 0, GLFW_FALSE);
}

#else

int main() {
	Engine::initAll(Window::Settings{
		.maximized = true,
		.multisamplingSamplesPerPixel = 16
	}, FONT);

	EngineUpdateLoop updateLoop(60.0f);
	MainLoop loop;

	while (updateLoop.isRunning()) {
		loop.update();
	}

	Engine::terminateAll();
}

#ifdef FINAL_RELEASE

#ifdef WIN32
#include <Windows.h>
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	return main();
}
#endif

#endif

#endif 
