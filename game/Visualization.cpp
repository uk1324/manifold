#include "Visualization.hpp"
#include <engine/Math/Color.hpp>
#include <game/Constants.hpp>
#include <engine/Window.hpp>
#include <imgui/imgui.h>
#include <engine/Input/Input.hpp>
#include <engine/Math/Angles.hpp>

Visualization::Visualization() 
	: renderer(GameRenderer::make())
	, noise(0) {

	Window::disableCursor();
}

Quat movementOnSphericalGeodesic(Vec3 pos, f32 angle, f32 distance) {
	using CalculationType = f64;
	const auto p = Vec3T<CalculationType>(pos);
	const auto a = CalculationType(atan2(pos.y, pos.x));
	const auto up = Vec3T<CalculationType>(0.0f, 0.0f, 1.0f);

	Vec3T<CalculationType> axis(0.0f, 1.0f, 0.0f);
	if (p != -up) {
		axis = cross(p, up).normalized();
	}

	const auto result = (QuatT<CalculationType>(-angle + a, pos) * QuatT<CalculationType>(distance, axis)).normalized();
	return Quat(
		f32(result.x),
		f32(result.y),
		f32(result.z),
		f32(result.w)
	);
}

void Visualization::update() {
	if (Input::isKeyDown(KeyCode::ESCAPE)) {
		Window::toggleCursor();
	}
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

	auto view = Mat4::identity;
	Vec3 cameraPosition(0.0f);
	{
		camera.update(Constants::dt);
		view = camera.viewMatrix();
		cameraPosition = camera.position;
	}

	// Could convert to Mat3 and just do a transpose.
	const auto cameraForward = (Vec4(Vec3::FORWARD, 0.0f) * view.inversed()).xyz().normalized();
	const auto aspectRatio = Window::aspectRatio();
	const auto projection = Mat4::perspective(PI<f32> / 2.0f, aspectRatio, 0.1f, 1000.0f);
	renderer.transform = projection * view;
	renderer.view = view;
	renderer.projection = projection;
	renderer.resizeBuffers(Vec2T<i32>(Window::size()));
	glViewport(0, 0, i32(Window::size().x), i32(Window::size().y));

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_BLEND);

	static f32 elapsed = 0.0f;
	elapsed += Constants::dt;
	positions.push_back(positionOnSphere);

	const auto rotationAxis = cross(positionOnSphere, movementDirection);
	const auto rotation = Quat(Constants::dt, rotationAxis);
	positionOnSphere *= rotation;
	//movementDirection *= rotation;

	positionOnSphere = positionOnSphere.normalized();
	movementDirection = cross(rotationAxis, positionOnSphere).normalized();

	const auto rotationSpeed = 5.0f;
	movementDirection *= Quat(noise.value2d(Vec2(0.0f, elapsed)) * Constants::dt * rotationSpeed, positionOnSphere);
	//movementDirectionAngle += noise.value2d(Vec2(0.0f, elapsed)) * Constants::dt;

	/*const auto movement = movementOnSphericalGeodesic(positionOnSphere, movementDirectionAngle, 0.01f);
	positionOnSphere *= movement;
	movementDirectionAngle += noise.value2d(Vec2(0.0f, elapsed)) * Constants::dt;

	for (i32 i = 0; i < i32(positions.size()) - 1; i++) {
		renderer.line(positions[i], positions[i + 1], 0.01f, Color3::GREEN);
	}*/
	for (i32 i = 0; i < i32(positions.size()) - 1; i++) {
		renderer.line(positions[i], positions[i + 1], 0.01f, Color3::GREEN);
	}
 	renderer.renderHemispheres();
	renderer.renderCyllinders();
	/*const auto soup = regularPolyhedronPolygonSoup(constView(cubeVertices), constView(cubeFaces), cubeVerticesPerFace);
	const auto dual = dualPolyhedron(soup);
	renderPolygonSoup(dual);*/

	/*renderer.cube(Color3::GREEN);
	renderer.renderCubes();*/
}

void Visualization::renderPolygonSoup(const PolygonSoup& polygonSoup) {
	const auto data = flatShadeConvexPolygonSoup(polygonSoup);
	auto& t = renderer.coloredShadingTriangles;
	const auto offset = t.currentIndex();
	for (i32 i = 0; i < data.positions.size(); i++) {
		t.addVertex(Vertex3Pnc{
			.position = data.positions[i],
			.normal = data.normals[i],
			.color = Vec4(Color3::GREEN, 1.0f),
		});
	}
	for (i32 i = 0; i < data.indices.size(); i++) {
		t.indices.push_back(data.indices[i]);
	}
	renderer.renderColoredShadingTriangles();
}
