#include "MainLoop.hpp"
#include <glad/glad.h>
#include <game/PlotUtils.hpp>
#include <engine/Math/Mat2.hpp>
#include <engine/Math/Angles.hpp>
#include <engine/Math/Constants.hpp>
#include <engine/Window.hpp>
#include <engine/Input/Input.hpp>
#include <imgui/implot.h>

const auto dt = 1.0f / 60.0f;

MainLoop::MainLoop()
	: renderer(Renderer::make()) {

	Window::disableCursor();
}

// https://trecs.se/torus.php
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

Vec3 torusTangentU(f32 u, f32 v, f32 r, f32 R) {
	return Vec3(
		-r * sin(u) * cos(v),
		r * cos(u),
		-r * sin(u) * sin(v)
	);
}

Vec3 torusTangentV(f32 u, f32 v, f32 r, f32 R) {
	return Vec3(
		-(R + r * cos(u)) * sin(v),
		0.0f,
		(R + r * cos(u)) * cos(v)
	);
}

Vec3 torusNormal(f32 u, f32 v, f32 r, f32 R) {
	return Vec3(
		cos(u) * cos(v),
		sin(u),
		cos(u) * sin(v)
	).normalized();
}
	
void MainLoop::update1() {
	if (Input::isKeyDown(KeyCode::ESCAPE)) {
		Window::toggleCursor();
	}

	if (Input::isKeyDown(KeyCode::X)) {
		Window::close();
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, Window::size().x, Window::size().y);

	const auto r = 0.4f;
	const auto R = 1.0f;
	{
		const auto startIndex = renderer.trianglesIndices.size();
		const auto size = 50;
		for (i32 vi = 0; vi < size; vi++) {
			for (i32 ui = 0; ui < size; ui++) {
				const auto u = f32(ui) / size * TAU<f32>;
				const auto v = f32(vi) / size * TAU<f32>;
				const auto p = torus(u, v, r, R);
				const auto n = torusNormal(u, v, r, R);
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
	//f32 speed = 0.01f;
	//Vec2 direction(0.0f);
	//if (Input::isKeyHeld(KeyCode::RIGHT)) { direction.x += 1.0f; }
	//if (Input::isKeyHeld(KeyCode::LEFT)) { direction.x -= 1.0f; }
	//if (Input::isKeyHeld(KeyCode::UP)) { direction.y += 1.0f; }
	//if (Input::isKeyHeld(KeyCode::DOWN)) { direction.y -= 1.0f; }

	Vec3 cameraUp = torusNormal(uvPosition.x, uvPosition.y, r, R);
	Vec3 forwardTangent = torusTangentU(uvPosition.x, uvPosition.y, r, R).normalized();
	Vec3 tangentSpaceForward =
		Quat(uvForwardAngle, cameraUp) *
		forwardTangent;
	tangentSpaceForward -= dot(tangentSpaceForward, cameraUp) * cameraUp;

	const auto uTangent = torusTangentU(uvPosition.x, uvPosition.y, r, R);
	const auto vTangent = torusTangentV(uvPosition.x, uvPosition.y, r, R);
	// Orthonormal basis for tangent space.
	Vec3 xAxis = uTangent.normalized();
	Vec3 yAxis = cross(uTangent, cameraUp).normalized();
	Vec2 ru = Vec2(dot(uTangent, xAxis), dot(uTangent, yAxis));
	Vec2 rv = Vec2(dot(vTangent, xAxis), dot(vTangent, yAxis));
	Vec2 f = Vec2(dot(tangentSpaceForward, xAxis), dot(tangentSpaceForward, yAxis));

	// Want to solve 
	// a1 ru + a2 rv = f
	// for a1, a2
	const auto coordinates = Mat2(ru, rv).inversed() * f;

	//uvPosition += direction.normalized() * speed;
	if (Input::isKeyHeld(KeyCode::W)) {
		uvPosition += coordinates * 0.01f;
	} 
	if (Input::isKeyHeld(KeyCode::S)) {
		uvPosition -= coordinates * 0.01f;
	}
	
	Vec3 cameraPosition = torus(uvPosition.x, uvPosition.y, r, R) + cameraUp * 0.2f;
	Vec3 cameraRight = cross(cameraUp, forwardTangent).normalized();
	Vec3 cameraForward = 
		Quat(uvForwardAngle, cameraUp) * 
		Quat(rightAxisAngle, cameraRight) *
		forwardTangent;
	const auto view = Mat4::lookAt(cameraPosition, cameraPosition + cameraForward, cameraUp);
	const auto aspectRatio = Window::aspectRatio();
	const auto projection = Mat4::perspective(PI<f32> / 2.0f, aspectRatio, 0.1f, 1000.0f);
	const auto viewProjection = projection * view;
	renderer.viewProjection = viewProjection;

	if (Window::isCursorEnabled()) {
		lastMousePosition = std::nullopt;
	} else {
		Vec2 mouseOffset(0.0f);
		if (lastMousePosition.has_value()) {
			mouseOffset = Input::cursorPosWindowSpace() - *lastMousePosition;
		} else {
			mouseOffset = Vec2(0.0f);
		}

		lastMousePosition = Input::cursorPosWindowSpace();

		// Angle change per pixel per second?
		float rotationSpeed = 0.1f;
		float movementSpeed = 1.0f;
		// x+ is right both in window space and in the used coordinate system.
		// The coordinate system is left handed so by applying the left hand rule a positive angle change turns the camera right.
		/*angleAroundUpAxis += mouseOffset.x * rotationSpeed * dt;*/
		uvForwardAngle += mouseOffset.x * rotationSpeed * dt;
		// Down is positive in window space and a positive rotation around the x axis rotates down.
		rightAxisAngle += mouseOffset.y * rotationSpeed * dt;

		uvForwardAngle = normalizeAngleZeroToTau(uvForwardAngle);
		rightAxisAngle = std::clamp(rightAxisAngle, -degToRad(89.0f), degToRad(89.0f));
	}

	uvPosition.x = fmod(uvPosition.x, TAU<f32>);
	uvPosition.y = fmod(uvPosition.y, TAU<f32>);
	uvPositions.push_back(uvPosition);
	ImGui::Begin("plot");
	if (ImPlot::BeginPlot("plot", ImVec2(-1.0f, -1.0f), ImPlotFlags_Equal)) {
		ImPlot::SetupAxesLimits(0.0f, TAU<f32>, 0.0f, TAU<f32>);
		//ImPlot::SetPlotLimits
		plotVec2Scatter("points", uvPositions);
		ImPlot::EndPlot();
	}
	ImGui::End();
	/*renderer.camera.up = cameraUp;
	renderer.camera.position = cameraPosition + cameraUp * 0.1f;*/
	{
		/*const auto view = camera.viewMatrix();
		const auto projection = Mat4::perspective(PI<f32> / 2.0f, aspectRatio, 0.1f, 1000.0f);
		return projection * view;*/
	}


	/*if (Window::isCursorEnabled()) {
		renderer.camera.lastMousePosition = std::nullopt;
	} else {
		renderer.camera.update(dt);
	}*/
	renderer.renderTriangles();
}


void MainLoop::update() {
	update1();
	return;
	if (Input::isKeyDown(KeyCode::ESCAPE)) {
		Window::toggleCursor();
	}

	if (Input::isKeyDown(KeyCode::X)) {
		Window::close();
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, Window::size().x, Window::size().y);

	//const auto r = 0.4f;
	//const auto R = 1.0f;
	//{
	//	const auto startIndex = renderer.trianglesIndices.size();
	//	const auto size = 50;
	//	for (i32 vi = 0; vi < size; vi++) {
	//		for (i32 ui = 0; ui < size; ui++) {
	//			const auto u = f32(ui) / size * TAU<f32>;
	//			const auto v = f32(vi) / size * TAU<f32>;
	//			const auto p = torus(u, v, r, R);
	//			const auto n = torusNormal(u, v, r, R);
	//			renderer.addVertex(Vertex3Pn{ .position = p, .normal = n });
	//		}
	//	}

	//	auto index = [&size](i32 ui, i32 vi) {
	//		// Wrap aroud
	//		if (ui == size) { ui = 0; }
	//		if (vi == size) { vi = 0; }

	//		return vi * size + ui;
	//		};
	//	for (i32 vi = 0; vi < size; vi++) {
	//		for (i32 ui = 0; ui < size; ui++) {
	//			const auto i0 = index(ui, vi);
	//			const auto i1 = index(ui + 1, vi);
	//			const auto i2 = index(ui + 1, vi + 1);
	//			const auto i3 = index(ui, vi + 1);
	//			renderer.addQuad(i0, i1, i2, i3);
	//		}
	//	}
	//}
	//f32 speed = 0.01f;
	//Vec2 direction(0.0f);
	//if (Input::isKeyHeld(KeyCode::RIGHT)) { direction.x += 1.0f; }
	//if (Input::isKeyHeld(KeyCode::LEFT)) { direction.x -= 1.0f; }
	//if (Input::isKeyHeld(KeyCode::UP)) { direction.y += 1.0f; }
	//if (Input::isKeyHeld(KeyCode::DOWN)) { direction.y -= 1.0f; }

	//Vec3 cameraUp = torusNormal(uvPosition.x, uvPosition.y, r, R);
	//Vec3 forwardTangent = torusTangentU(uvPosition.x, uvPosition.y, r, R).normalized();
	//Vec3 tangentSpaceForward =
	//	Quat(uvForwardAngle, cameraUp) *
	//	forwardTangent;
	//tangentSpaceForward -= dot(tangentSpaceForward, cameraUp) * cameraUp;

	//const auto uTangent = torusTangentU(uvPosition.x, uvPosition.y, r, R);
	//const auto vTangent = torusTangentV(uvPosition.x, uvPosition.y, r, R);
	//// Orthonormal basis for tangent space.
	//Vec3 xAxis = uTangent.normalized();
	//Vec3 yAxis = cross(uTangent, cameraUp).normalized();
	//Vec2 ru = Vec2(dot(uTangent, xAxis), dot(uTangent, yAxis));
	//Vec2 rv = Vec2(dot(vTangent, xAxis), dot(vTangent, yAxis));
	//Vec2 f = Vec2(dot(tangentSpaceForward, xAxis), dot(tangentSpaceForward, yAxis));

	//// Want to solve 
	//// a1 ru + a2 rv = f
	//// for a1, a2
	//const auto coordinates = Mat2(ru, rv).inversed() * f;

	////uvPosition += direction.normalized() * speed;
	//if (Input::isKeyHeld(KeyCode::W)) {
	//	uvPosition += coordinates * 0.01f;
	//}
	//if (Input::isKeyHeld(KeyCode::S)) {
	//	uvPosition -= coordinates * 0.01f;
	//}

	//Vec3 cameraPosition = torus(uvPosition.x, uvPosition.y, r, R) + cameraUp * 0.2f;
	//Vec3 cameraRight = cross(cameraUp, forwardTangent).normalized();
	//Vec3 cameraForward =
	//	Quat(uvForwardAngle, cameraUp) *
	//	Quat(rightAxisAngle, cameraRight) *
	//	forwardTangent;
	//const auto view = Mat4::lookAt(cameraPosition, cameraPosition + cameraForward, cameraUp);
	//const auto aspectRatio = Window::aspectRatio();
	//const auto projection = Mat4::perspective(PI<f32> / 2.0f, aspectRatio, 0.1f, 1000.0f);
	//const auto viewProjection = projection * view;
	//renderer.viewProjection = viewProjection;

	//if (Window::isCursorEnabled()) {
	//	lastMousePosition = std::nullopt;
	//} else {
	//	Vec2 mouseOffset(0.0f);
	//	if (lastMousePosition.has_value()) {
	//		mouseOffset = Input::cursorPosWindowSpace() - *lastMousePosition;
	//	} else {
	//		mouseOffset = Vec2(0.0f);
	//	}

	//	lastMousePosition = Input::cursorPosWindowSpace();

	//	// Angle change per pixel per second?
	//	float rotationSpeed = 0.1f;
	//	float movementSpeed = 1.0f;
	//	// x+ is right both in window space and in the used coordinate system.
	//	// The coordinate system is left handed so by applying the left hand rule a positive angle change turns the camera right.
	//	/*angleAroundUpAxis += mouseOffset.x * rotationSpeed * dt;*/
	//	uvForwardAngle += mouseOffset.x * rotationSpeed * dt;
	//	// Down is positive in window space and a positive rotation around the x axis rotates down.
	//	rightAxisAngle += mouseOffset.y * rotationSpeed * dt;

	//	uvForwardAngle = normalizeAngleZeroToTau(uvForwardAngle);
	//	rightAxisAngle = std::clamp(rightAxisAngle, -degToRad(89.0f), degToRad(89.0f));
	//}
	
	if (Input::isKeyHeld(KeyCode::A)) { uvForwardAngle += 1.0f * dt; }
	if (Input::isKeyHeld(KeyCode::D)) { uvForwardAngle -= 1.0f * dt; }

	if (Input::isKeyDown(KeyCode::W)) {
		uvVelocity = Vec2::oriented(uvForwardAngle);
	} else if (Input::isKeyHeld(KeyCode::W)) {
		uvForwardAngle = uvVelocity.angle();
			//uvVelocity += 
			//uvPosition += uvVelocity
	} else {
		uvVelocity = Vec2(0.0f);
	}

	/*if (Input::isKeyHeld(KeyCode::UP)) { direction.y += 1.0f; }
	if (Input::isKeyHeld(KeyCode::DOWN)) { direction.y -= 1.0f; }*/

	uvPosition.x = fmod(uvPosition.x, TAU<f32>);
	uvPosition.y = fmod(uvPosition.y, TAU<f32>);
	uvPositions.push_back(uvPosition);
	ImGui::Begin("plot");
	if (ImPlot::BeginPlot("plot", ImVec2(-1.0f, -1.0f), ImPlotFlags_Equal)) {
		ImPlot::SetupAxesLimits(0.0f, TAU<f32>, 0.0f, TAU<f32>);
		//ImPlot::SetPlotLimits
		plotVec2Scatter("points", uvPositions);
		ImPlot::EndPlot();
	}
	ImGui::End();
	/*renderer.camera.up = cameraUp;
	renderer.camera.position = cameraPosition + cameraUp * 0.1f;*/
	{
		/*const auto view = camera.viewMatrix();
		const auto projection = Mat4::perspective(PI<f32> / 2.0f, aspectRatio, 0.1f, 1000.0f);
		return projection * view;*/
	}


	/*if (Window::isCursorEnabled()) {
		renderer.camera.lastMousePosition = std::nullopt;
	} else {
		renderer.camera.update(dt);
	}*/
	//renderer.renderTriangles();
}
