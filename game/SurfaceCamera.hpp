#pragma once

#include <engine/Math/Mat4.hpp>
#include <engine/Math/Quat.hpp>
#include <engine/Math/Vec2.hpp>
#include <engine/Math/Angles.hpp>
#include <optional>
#include <imgui/imgui.h>
#include <engine/Input/Input.hpp>
#include <engine/Window.hpp>
#include <game/Surfaces/RectParametrization.hpp>
#include <game/Utils.hpp>

struct SufaceCamera {
	// In uv space
	Vec2 uvPosition = Vec2(0.1f);
	f32 uvForwardAngle = 0.0f;

	// In 3d space
	f32 angleToTangentPlane = 0.0f;
	f32 height = 0.2f;

	bool normalFlipped = false;
	std::optional<Vec2> lastMousePosition;

	f32 normalSign() const;

	template<typename Surface>
	Mat4 update(const Surface& surface, f32 dt);
	Vec3 cameraPosition(Vec3 position, Vec3 normalAtUvPosition) const;
	template<typename Surface>
	Vec3 cameraPosition(const Surface& surface) const;
};

template<typename Surface>
Mat4 SufaceCamera::update(const Surface& surface, f32 dt) {
	const auto normalSign = this->normalSign();
	Vec3 cameraUp = surface.normal(uvPosition.x, uvPosition.y) * normalSign;
	const auto uTangent = surface.tangentU(uvPosition.x, uvPosition.y);
	const auto vTangent = surface.tangentV(uvPosition.x, uvPosition.y);

	const auto forwardTangent = (cos(uvForwardAngle) * uTangent + sin(uvForwardAngle) * vTangent).normalized();
	Vec3 cameraPosition = this->cameraPosition(surface.position(uvPosition.x, uvPosition.y), cameraUp);
	Vec3 cameraRight = cross(cameraUp, forwardTangent).normalized();
	Vec3 cameraForward =
		Quat(angleToTangentPlane, cameraRight) *
		forwardTangent;
	const auto view = Mat4::lookAt(cameraPosition, cameraPosition + cameraForward, cameraUp);

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

			auto result = 1.0f / scale;
			result *= mouseOffset.x * rotationSpeed * normalSign;
			return result;
		};
		/*
		rhs can get quite big so if the step size is too big (the user moves the mouse too fast) then things glitch out. The mouse might spin multiple times in a single step and it look like it just teleported. This happens for example with the torus and n = 1.
		*/
		// This is what rhs looks like.
		//if (ImPlot::BeginPlot("rhs")) {
		//	auto rhs = [&](f32 a, f32 _) {
		//		const auto f = cos(a) * uTangent + sin(a) * vTangent;
		//		const auto df = -sin(a) * uTangent + cos(a) * vTangent;
		//		const auto scale = ((df * f.length() - f * (dot(f, df) / f.length())) / f.lengthSquared()).length();
		//		return 1.0f / scale;
		//	};

		//	ImPlot::SetupAxesLimits(0.0f, TAU<f32>, 0.0f, 40.0f);
		//	std::vector<f32> xs, ys;
		//	const auto n = 200;
		//	for (i32 i = 0; i < 200; i++) {
		//		const auto x = lerp(0.0f, TAU<f32>, f32(i) / (n - 1));
		//		const auto y = rhs(x, 0.0f);
		//		xs.push_back(x);
		//		ys.push_back(y);
		//	}
		//	ImPlot::PlotLine("rhs(a)", xs.data(), ys.data(), n);
		//	ImPlot::EndPlot();
		//}


		i32 n = 5;
		for (i32 i = 0; i < n; i++) {
			uvForwardAngle = rungeKutta4Step(rhs, uvForwardAngle, 0.0f, dt / n);
		}

		angleToTangentPlane += mouseOffset.y * rotationSpeed * dt;

		uvForwardAngle = normalizeAngleZeroToTau(uvForwardAngle);
		angleToTangentPlane = std::clamp(angleToTangentPlane, -degToRad(89.0f), degToRad(89.0f));
	}


	auto movementRhs = [&](Vec4 state, f32 _) {
		const auto symbols = surface.christoffelSymbols(state.x, state.y);
		Vec2 velocity(state.z, state.w);

		return Vec4(
			velocity.x,
			velocity.y,
			-dot(velocity, symbols.x * velocity),
			-dot(velocity, symbols.y * velocity)
		);
	};

	// Solutions to the geodesic equation are constant speed
	// https://www.ams.jhu.edu/~mmiche18/120a.1.10w/lectures.html
	// Lecture 25: https://www.ams.jhu.edu/~mmiche18/120a.1.10w/LECTURES/Math120A_Lecture_25.pdf

	// Unrelated, but this is a formulation that uses time instead of arclength. 
	// https://en.wikipedia.org/wiki/Geodesics_in_general_relativity#Equivalent_mathematical_expression_using_coordinate_time_as_parameter
	// This only makes sense in spacetime.

	//Vec2 velocityDirection(0.0f);

	//if (Input::isKeyHeld(KeyCode::W)) {
	//	velocityDirection += Vec2::oriented(uvForwardAngle);
	//}
	//if (Input::isKeyHeld(KeyCode::S)) {
	//	velocityDirection += -Vec2::oriented(uvForwardAngle);
	//}
	

	Vec3 movementDirection(0.0f);
	const auto forward = (cos(uvForwardAngle) * uTangent + sin(uvForwardAngle) * vTangent).normalized();
	const auto right = cross(cameraUp, forward).normalized();
	{
		// Recomputing this, because uvForwardAngle changed.
		if (Input::isKeyHeld(KeyCode::W)) {
			movementDirection += forward;
		}
		if (Input::isKeyHeld(KeyCode::S)) {
			movementDirection -= forward;
		}
		if (Input::isKeyHeld(KeyCode::D)) {
			movementDirection += right;
		}
		if (Input::isKeyHeld(KeyCode::A)) {
			movementDirection -= right;
		}
	}
	
	const auto angleBetweenForwardAndMovement = atan2(
		dot(movementDirection, right),
		dot(movementDirection, forward)
	);

	if (movementDirection != Vec3(0.0f)) {
		Vec2 velocity = vectorInTangentSpaceBasis(movementDirection, uTangent, vTangent, cameraUp);
		//Vec2 velocity = velocityDirection;
		const auto v = (velocity.x * uTangent + velocity.y * vTangent).length();
		// Unit speed initial condition.
		velocity /= v;

		const auto a = movementRhs(Vec4(uvPosition.x, uvPosition.y, velocity.x, velocity.y), 0.0f);
		ImGui::Text("%g", Vec2(a.z, a.w).length());

		i32 n = 5;
		Vec4 state(uvPosition.x, uvPosition.y, velocity.x, velocity.y);
		for (i32 i = 0; i < n; i++) {
			state = rungeKutta4Step(movementRhs, state, 0.0f, dt / n);
		}

		uvPosition = Vec2(state.x, state.y);
		const auto uTangentNew = surface.tangentU(uvPosition.x, uvPosition.y);
		const auto vTangentNew = surface.tangentV(uvPosition.x, uvPosition.y);
		const Vec3 normalNew = surface.normal(uvPosition.x, uvPosition.y) * normalSign;

		const auto newMovementDirection = state.z * uTangentNew + state.w * vTangentNew;
		const auto newForwardDirection = Quat(-angleBetweenForwardAndMovement, normalNew) * newMovementDirection;
		const Vec2 newForwardUv = vectorInTangentSpaceBasis(newForwardDirection, uTangentNew, vTangentNew, normalNew);

		uvForwardAngle = newForwardUv.angle();
		//const auto v2 = (state.z * uTangent + state.w * vTangent).length();
		//ImGui::Text("%g", v2);

		//uvForwardAngle = Vec2(state.z, state.w).angle();
	}

	auto wrapToRange = [](f32 value, f32 min, f32 max) {
		value -= min;
		const auto range = max - min;
		value = fmod(value, range);
		if (value < 0.0f) {
			value += range;
		}
		value += min;
		return value;
	};

	auto handleConnectivity = [&wrapToRange](f32 value, f32 min, f32 max, SquareSideConnectivity connectivity) -> f32 {
		switch (connectivity) {
			using enum SquareSideConnectivity;
		case NONE:
			return std::clamp(value, min, max);

		case NORMAL:
			return wrapToRange(value, min, max);

		case REVERSED:
			// TODO:
			return value;
		};

		return value;
	};

	uvPosition.x = handleConnectivity(uvPosition.x, surface.uMin, surface.uMax, surface.uConnectivity);
	uvPosition.y = handleConnectivity(uvPosition.y, surface.vMin, surface.vMax, surface.vConnectivity);

	return view;
}

template<typename Surface>
Vec3 SufaceCamera::cameraPosition(const Surface& surface) const {
	return cameraPosition(surface.position(uvPosition.x, uvPosition.y), surface.normal(uvPosition.x, uvPosition.y) * normalSign());
}
