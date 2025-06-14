#include "MainLoop.hpp"
#include <gfx/ShaderManager.hpp>
#include <engine/Window.hpp>
#include <imgui/imgui.h>



MainLoop::MainLoop()
	//: game(Game::make())
	: renderer(GameRenderer::make()) {
    //ImGui::GetIO().ConfigDpiScaleViewports = true;
}

void onWindowSizeChange(Vec2 size) {
    //return;
    if (size.x <= 0 || size.y <= 0) {
        return;
    }
    auto& style = ImGui::GetStyle();
    style = ImGuiStyle();
    style.WindowRounding = 5.0f;

    auto& io = ImGui::GetIO();
    const auto fontSize = ImGui::GetFontSize();
    const auto displaySize = io.DisplaySize.x;
    // fontSize * scale = displaySize * fontDisplayRatio.
    // scale = displaySize * fontDisplayRatio / fontSize
    const auto fontDisplayRatio = 0.03f;
    const auto scale = displaySize * fontDisplayRatio / fontSize;
    style.ScaleAllSizes(scale);
    style.FontScaleDpi = scale;
    //io.FontGlobalScale = scale;
    //style.FontScale = scale;
    /*const auto fontPath = "engine/assets/fonts/RobotoMono-Regular.ttf";
    io.Fonts->AddFontFromFileTTF(fontPath, io.DisplaySize.x * 0.03f);*/

    // Reset all ImGui windows
    /*reset_imgui_style();
    float relative_scale_reset = static_cast<float>(resolution_x) / last_resolution_x;
    for (const auto& viewport : ImGui::GetCurrentContext()->Viewports)
    {
        ImGui::ScaleWindowsInViewport(viewport, relative_scale_reset);
    }*/

    // Scale them to the appropriate size
    /*float relative_scale = static_cast<float>(x) / resolution_x;
    for (const auto& viewport : ImGui::GetCurrentContext()->Viewports)
    {
        ImGui::ScaleWindowsInViewport(viewport, relative_scale);
    }
    ImGui::GetStyle().ScaleAllSizes(relative_scale);
    ImGui::GetIO().FontGlobalScale = relative_scale;*/
    //ImGui::GetIO().FontGlobalScale = ;

    /*last_resolution_x = x;
    last_resolution_y = y;*/
}

void MainLoop::update() {
	ShaderManager::update();

    
	static std::optional<Vec2> windowSize;
    if (!windowSize.has_value()) {
        windowSize = Window::size();
    } else {
        const auto currentWindowSize = Window::size();
        if (currentWindowSize != windowSize) {
            //onWindowSizeChange(currentWindowSize);
        }
        windowSize = currentWindowSize;
    }
	//game.update(renderer);
	minesweeper.update(renderer);
}