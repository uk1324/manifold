#include "MainLoop.hpp"
#include <glad/glad.h>
#include <engine/Math/Constants.hpp>
#include <engine/Window.hpp>
#include <engine/Input/Input.hpp>

const auto dt = 1.0f / 60.0f;

MainLoop::MainLoop()
	: renderer(Renderer::make()) {

	Window::disableCursor();
}

Vec3 torus(f32 u, f32 v, f32 r, f32 R) {
	return Vec3(
		(R + r * cos(u)) * cos(v),
		r * sin(u),
		(R + r * cos(u)) * sin(v)
	);
	/*
	r_u = [-r sin(u) cos(v), -r sin(u) sin(v), rcos(u)]
	r_v = [-(R + r cos(u))sin(v), (R + r cos(u))cos(v), 0]
	*/
}

Vec3 torusNormal(f32 u, f32 v, f32 r, f32 R) {
	return Vec3(
		cos(u) * cos(v),
		sin(u),
		cos(u) * sin(v)
	);
}
	
void MainLoop::update() {
	if (Input::isKeyDown(KeyCode::ESCAPE)) {
		Window::toggleCursor();
	}

	if (Input::isKeyDown(KeyCode::X)) {
		Window::close();
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, Window::size().x, Window::size().y);

	{
		const auto startIndex = renderer.trianglesIndices.size();
		const auto size = 50;
		const auto r = 0.4f;
		const auto R = 1.0f;
		for (i32 vi = 0; vi < size; vi++) {
			for (i32 ui = 0; ui < size; ui++) {
				const auto u = f32(ui) / size * TAU<f32>;
				const auto v = f32(vi) / size * TAU<f32>;
				const auto p = torus(u, v, r, R);
				const auto n = torusNormal(u, v, r, R).normalized();
				renderer.addVertex(Vertex3Pn{ .position = p, .normal = n });
			}
		}

		auto index = [&size](i32 ui, i32 vi) {
			// Wrap aroud
			if (ui == size) { ui = 0; } 
			if (vi == size) { vi = 0; }

			return vi * size + ui;
		};
		for (i32 vi = 0; vi < size; vi++) {
			for (i32 ui = 0; ui < size; ui++) {
				const auto i0 = index(ui, vi);
				const auto i1 = index(ui + 1, vi);
				const auto i2 = index(ui + 1, vi + 1);
				const auto i3 = index(ui, vi + 1);
				renderer.addQuad(i0, i1, i2, i3);
			}
		}
	}

	if (Window::isCursorEnabled()) {
		renderer.camera.lastMousePosition = std::nullopt;
	} else {
		renderer.camera.update(dt);
	}
	renderer.renderTriangles();
}
