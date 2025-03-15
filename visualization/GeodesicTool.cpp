#include "GeodesicTool.hpp"
#include <engine/Input/Input.hpp>
#include <game/SurfaceSwitch.hpp>
#include <engine/Math/Color.hpp>
#include <game/RayIntersection.hpp>
#include <game/Utils.hpp>
#include <engine/Math/OdeIntegration/RungeKutta4.hpp>

void GeodesicTool::update(
	Vec3 cameraPosition, 
	Vec3 cameraForward, 
	std::vector<MeshIntersection>& intersections,
	const Surfaces& surfaces,
	Renderer& renderer) {

	const auto initialVelocityVectorLength = 0.25f;
	const auto initialPositionPos = surfaces.position(initialPositionUv);
	const auto initialPositionTangentU = surfaces.tangentU(initialPositionUv);
	const auto initialPositionTangentV = surfaces.tangentV(initialPositionUv);
	const auto initialVelocityVectorEndPosition =
		initialPositionPos + 
		initialVelocityVectorLength * 
		(initialVelocityUv.x * initialPositionTangentU + initialVelocityUv.y * initialPositionTangentV).normalized();

	struct TangentPlanePosition {
		Vec3 inSpace;
		Vec2 inUvCoordinates;
	};
	std::optional<TangentPlanePosition> tangentPlaneIntersection;

	const auto tangentPlaneNormal = cross(initialPositionTangentU, initialPositionTangentV);
	{
		const auto intersectionT = rayPlaneIntersection(cameraPosition, cameraForward, initialPositionPos, tangentPlaneNormal);
		if (intersectionT.has_value()) {
			const auto intersection = cameraPosition + *intersectionT * cameraForward;
			// TODO: Could allow snapping to the grid.
			tangentPlaneIntersection = TangentPlanePosition{
				.inSpace = intersection,
				.inUvCoordinates = vectorInTangentSpaceBasis(
					intersection - initialPositionPos,
					initialPositionTangentU,
					initialPositionTangentV,
					tangentPlaneNormal
				)
			};
		}
	}

	if (Input::isMouseButtonDown(MouseButton::LEFT)) {
		sortIntersectionsByDistanceToCamera(intersections);
		const auto grabDistance = 0.06f;
		if (checkIfPointGotGrabbed(initialPositionPos, intersections)) {
			grabbed = Grabbed::POSITION;
		}
		if (grabbed == Grabbed::NONE && 
			tangentPlaneIntersection &&
			tangentPlaneIntersection->inSpace.distanceTo(initialVelocityVectorEndPosition) < grabDistance) {
			grabbed = Grabbed::VELOCITY;
		}
	}
	if (Input::isMouseButtonUp(MouseButton::LEFT)) {
		grabbed = Grabbed::NONE;
	}
	switch (grabbed) {
		using enum Grabbed;
	case NONE:
		break;
	case POSITION: {
		updateGrabbedPoint(initialPositionUv, initialPositionPos, intersections);
		break;
	}

	case VELOCITY: {
		if (tangentPlaneIntersection.has_value()) {
			// Using this, if the user points above the horizion of the tangent plane it no longer detects the direction. I want just the direction so to always find a direction I could theoretically create a parallel tangent plane above the user and also add points at infinity so that there is always an intersection.
			// The issue is that this depends on both the angle the user makes with the tangent plane and the y position relative to the tangent plane so it would probably also change depending on the position of the parallel tangent plane.
			// One alternative would be to choose the direction at inifinity if the user points above the tangent plane.
			initialVelocityUv = tangentPlaneIntersection->inUvCoordinates.normalized();
			/*initialVelocityUv = vectorInTangentSpaceBasis(
				cameraForward,
				initialPositionTangentU,
				initialPositionTangentV,
				tangentPlaneNormal
			);*/
		} else {
			Vec2 intersectionWithTangentPlaneAtInfinity = vectorInTangentSpaceBasis(
				cameraForward,
				initialPositionTangentU,
				initialPositionTangentV,
				tangentPlaneNormal
			);
			// The third coorindate is zero.

			/*
			We have the initialPosition and the cameraPosition.
			In this case the cameraDirection is parallel to the tangent plane so we can consider things as happening in the projective plane.
			The guy at cameraPosition is pointing in cameraDirection so it intersects at infinity at (cameraDirection, 0).
			Now we want to know the what direction does initialPosition need to point to also point at the point at infinity. 
			The answer is that it also has to point in the direction cameraDirection, because 2 lines intersect at a point at infinity iff they point in the same direction.
			*/
			initialVelocityUv = intersectionWithTangentPlaneAtInfinity;
		}
		break;
	}

	}

	const auto initialPosition = surfaces.position(initialPositionUv);
	const auto initialVelocity =
		(surfaces.tangentU(initialPositionUv) * initialVelocityUv.x +
			surfaces.tangentV(initialPositionUv) * initialVelocityUv.y).normalized();
	renderer.sphere(initialPosition, 0.015f, Color3::RED);
	const auto radius = 0.01f;
	const auto coneRadius = radius * 2.5f;
	const auto coneLength = coneRadius * 2.0f;
	renderer.arrowStartEnd(
		initialPosition,
		initialPosition + initialVelocity * initialVelocityVectorLength,
		radius,
		coneRadius,
		coneLength,
		Color3::WHITE,
		Color3::RED
	);

	#define U(name) integrateGeodesic(surfaces.name, renderer); break;
	SURFACE_SWITCH(surfaces.selected, U);
	#undef U
}

void GeodesicTool::integrateGeodesic(const RectParametrization auto& surface, Renderer& renderer) {
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

	const auto geodesicLength = 15.0f;
	const auto steps = 200;
	const auto dl = geodesicLength / steps;

	Vec2 position = initialPositionUv;
	Vec2 velocity = Vec2::oriented(initialVelocityUv.angle());
	for (i32 i = 0; i < steps; i++) {
		const auto uTangent = surface.tangentU(position.x, position.y);
		const auto vTangent = surface.tangentV(position.x, position.y);
		const auto v = (velocity.x * uTangent + velocity.y * vTangent).length();
		velocity /= v;
		i32 n = 5;
		Vec4 state(position.x, position.y, velocity.x, velocity.y);
		for (i32 j = 0; j < n; j++) {
			state = rungeKutta4Step(movementRhs, state, 0.0f, dl / n);
		}
		const auto newPosition = Vec2(state.x, state.y);
		renderer.line(surface.position(position.x, position.y), surface.position(newPosition.x, newPosition.y), 0.01f, Color3::RED);
		const auto tangent = Vec2(state.z, state.w);
		position = newPosition;
		velocity = tangent;
	}
}
