#include "MainLoop.hpp"
#include <glad/glad.h>
#include <engine/Math/Interpolation.hpp>
#include <engine/Math/OdeIntegration/RungeKutta4.hpp>
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
		(R + r * cos(v)) * cos(u),
		(R + r * cos(v)) * sin(u),
		r * sin(v)
	);
}

Vec3 torusTangentU(f32 u, f32 v, f32 r, f32 R) {
	return Vec3(
		-(R + r * cos(v)) * sin(u),
		(R + r * cos(v)) * cos(u),
		0.0f
	);
}

Vec3 torusTangentV(f32 u, f32 v, f32 r, f32 R) {
	return Vec3(
		-r * sin(v) * cos(u),
		-r * sin(v) * sin(u),
		r * cos(v)
	);
}

Vec3 torusNormal(f32 u, f32 v, f32 r, f32 R) {
	return Vec3(
		cos(u) * cos(v),
		sin(u) * cos(v),
		sin(v)
	).normalized();
}
	
struct ChristoffelSymbols {
	Mat2 x;
	Mat2 y;
};

ChristoffelSymbols torusChristoffelSymbols(f32 u, f32 v, f32 r, f32 R) {
	const auto a = -r * sin(v) / (R + r * cos(v));
	return {
		.x = Mat2(Vec2(0, a), Vec2(a, 0)),
		.y = Mat2(Vec2((R + r * cos(v)) * sin(v) / r, 0.0), Vec2(0.0f, 0.0f)),
	};
}

void renderClosedParametrizationOfRectangle(
	Renderer& renderer, 
	auto position, 
	auto normal,
	f32 uMin, f32 uMax, f32 vMin, f32 vMax) {
	const auto r = 0.4f;
	const auto R = 1.0f;
	{
		const auto startIndex = renderer.trianglesIndices.size();
		const auto size = 50;
		for (i32 vi = 0; vi < size; vi++) {
			for (i32 ui = 0; ui < size; ui++) {
				const auto u = lerp(uMin, uMax, f32(ui) / size);
				const auto v = lerp(vMin, vMax, f32(vi) / size);
				const auto p = position(u, v);
				const auto n = normal(u, v);
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
}

void updatePosition(
	Vec2& uvPosition, f32& uvForwardAngle,
	Vec3 tangentSpaceForward,
	Vec3 uTangent,
	Vec3 vTangent) {
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

	const auto r = 0.4f;
	const auto R = 10.0f;
	renderClosedParametrizationOfRectangle(
		renderer,
		[&](f32 u, f32 v) { return torus(u, v, r, R); },
		[&](f32 u, f32 v) { return torusNormal(u, v, r, R); },
		0.0f, TAU<f32>,
		0.0f, TAU<f32>
	);

	Vec3 cameraUp = torusNormal(uvPosition.x, uvPosition.y, r, R);
	const auto uTangent = torusTangentU(uvPosition.x, uvPosition.y, r, R);
	const auto vTangent = torusTangentV(uvPosition.x, uvPosition.y, r, R);
	const auto forwardTangent = (cos(uvForwardAngle) * uTangent + sin(uvForwardAngle) * vTangent).normalized();

	Vec3 cameraPosition = torus(uvPosition.x, uvPosition.y, r, R) + cameraUp * 0.2f;
	Vec3 cameraRight = cross(cameraUp, forwardTangent).normalized();
	Vec3 cameraForward =
		Quat(rightAxisAngle, cameraRight) *
		forwardTangent;
	const auto view = Mat4::lookAt(cameraPosition, cameraPosition + cameraForward, cameraUp);
	const auto aspectRatio = Window::aspectRatio();
	const auto projection = Mat4::perspective(PI<f32> / 2.0f, aspectRatio, 0.1f, 1000.0f);
	const auto swapYZ = Mat4(Mat3(Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f), Vec3(0.0f, 1.0f, 0.0f)));
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

		float rotationSpeed = 0.1f;
		/*
		I want to calculate by how much to change the uvForwardAngle so that the forward direction changes by a given amount

		The forward direction is
		F(a) = (cos(a) x_u + sin(a) x_v).normalized()
		from this i can calculate da as a function of |dF|

		In general if we have a vector x(t) then
		(x/|x|)' = (x'|x| + x |x|') / |x|^2

		|x|' can be calculated be rewriting it as sqrt(<x, x>)' and applying the product rule we get:
		1/2 sqrt(<x, x>) * 2<x, x'> = <x, x'> / |x|

		In this case x(a) = cos(a) x_u + sin(a) x_v so
		x'(a) = -sin(a) x_u + cos(a) x_v
		*/

		// dF/dA = rhs
		auto rhs = [&](f32 a, f32 _) {
			const auto f = cos(a) * uTangent + sin(a) * vTangent;
			const auto df = -sin(a) * uTangent + cos(a) * vTangent;

			// this is the unsimplified form
			const auto scale = ((df * f.length() - f * (dot(f, df) / f.length())) / f.lengthSquared()).length();
			//const auto scale = (df + f * (dot(f, df)/ pow(f.length(), 3))).length();

			return mouseOffset.x * rotationSpeed / scale;
		};
		/*
		rhs can get quite big so if the step size is too big (the user moves the mouse too fast) then things glitch out. The mouse might spin multiple times in a single step and it look like it just teleported. This happens for example with the torus and n = 1.
		*/
		// This is what rhs looks like.
		/*if (ImPlot::BeginPlot("rhs")) {
			auto rhs = [&](f32 a, f32 _) {
				const auto f = cos(a) * uTangent + sin(a) * vTangent;
				const auto df = -sin(a) * uTangent + cos(a) * vTangent;
				const auto scale = ((df * f.length() - f * (dot(f, df) / f.length())) / f.lengthSquared()).length();
				return 1.0f / scale;
			};

			ImPlot::SetupAxesLimits(0.0f, TAU<f32>, 0.0f, 40.0f);
			std::vector<f32> xs, ys;
			const auto n = 200;
			for (i32 i = 0; i < 200; i++) {
				const auto x = lerp(0.0f, TAU<f32>, f32(i) / (n - 1));
				const auto y = rhs(x, 0.0f);
				xs.push_back(x);
				ys.push_back(y);
			}
			ImPlot::PlotLine("rhs(a)", xs.data(), ys.data(), n);
			ImPlot::EndPlot();
		}*/


		i32 n = 5;
		for (i32 i = 0; i < n; i++) {
			uvForwardAngle = rungeKutta4Step(rhs, uvForwardAngle, 0.0f, dt / n);
		}

		//const auto f = cos(uvForwardAngle) * uTangent + sin(uvForwardAngle) * vTangent;
		//const auto df = -sin(uvForwardAngle) * uTangent + cos(uvForwardAngle) * vTangent;

		//// this is the unsimplified form
		//const auto scale = ((df * f.length() - f * (dot(f, df) / f.length())) / f.lengthSquared()).length();
		////const auto scale = (df + f * (dot(f, df)/ pow(f.length(), 3))).length();

 	//	const auto dFLength = mouseOffset.x * rotationSpeed * dt;
		//const auto da = dFLength / scale;
		//uvForwardAngle += da;
		/*uvForwardAngle += mouseOffset.x * rotationSpeed * dt / 
			(-sin(uvForwardAngle) * uTangent + cos(uvForwardAngle) * vTangent).length();*/
		rightAxisAngle += mouseOffset.y * rotationSpeed * dt;

		uvForwardAngle = normalizeAngleZeroToTau(uvForwardAngle);
		rightAxisAngle = std::clamp(rightAxisAngle, -degToRad(89.0f), degToRad(89.0f));
	}
	
	/*if (Input::isKeyHeld(KeyCode::A)) { uvForwardAngle += 1.0f * dt; }
	if (Input::isKeyHeld(KeyCode::D)) { uvForwardAngle -= 1.0f * dt; }*/


	if (Input::isKeyHeld(KeyCode::W)) {
		const auto symbols = torusChristoffelSymbols(uvPosition.x, uvPosition.y, r, R);
		Vec2 velocity = Vec2::oriented(uvForwardAngle);
		velocity.x -= dot(velocity, symbols.x * velocity) * dt;
		velocity.y -= dot(velocity, symbols.y * velocity) * dt;
		const auto v = (velocity.x * uTangent + velocity.y * vTangent).length();
		velocity /= v;
		uvPosition += velocity * dt;
		uvForwardAngle = velocity.angle();
	}

	/*if (Input::isKeyHeld(KeyCode::UP)) { direction.y += 1.0f; }
	if (Input::isKeyHeld(KeyCode::DOWN)) { direction.y -= 1.0f; }*/

	uvPosition.x = fmod(uvPosition.x, TAU<f32>);
	uvPosition.y = fmod(uvPosition.y, TAU<f32>);
	if (uvPosition.x < 0.0f) { uvPosition.x += TAU<f32>; }
	if (uvPosition.y < 0.0f) { uvPosition.y += TAU<f32>; }
	//Vec2 p = uvPosition;
	//if (p.x > PI<f32>) {
	//	p.x -= TAU<f32>;
	//}
	//if (p.y > PI<f32>) {
	//	p.y -= TAU<f32>;
	//}
	//uvPositions.push_back(p);
	ImGui::Begin("plot");
	if (ImPlot::BeginPlot("plot", ImVec2(-1.0f, -1.0f), ImPlotFlags_Equal)) {

		//ImPlot::SetupAxesLimits(-PI<f32>, PI<f32>, -PI<f32>, PI<f32>);
		ImPlot::SetupAxesLimits(0.0f, TAU<f32>, 0.0f, TAU<f32>);
		Vec2 forward = uvPosition + Vec2::oriented(uvForwardAngle) * 0.3f;
		f32 xs[] = { uvPosition.x, forward.x };
		f32 ys[] = { uvPosition.y, forward.y };
		ImPlot::PlotLine("arrow", xs, ys, 2);
		//plotVec2Scatter("points", uvPositions);
		ImPlot::EndPlot();
	}
	ImGui::End();

	renderer.renderTriangles();

	ImPlot::ShowDemoWindow();

}
