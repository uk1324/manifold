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
#include <game/Surface.hpp>

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

	Mat4 update(const Surface& surface, f32 dt);
	Vec3 cameraPosition(Vec3 position, Vec3 normalAtUvPosition) const;
	Vec3 cameraPosition(const Surface& surface) const;
};