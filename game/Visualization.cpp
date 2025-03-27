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

Quat exponentialMap(Vec3 vectorPart) {
	const auto distance = vectorPart.length();
	const auto v = vectorPart / distance;
	return Quat(v.x * sin(distance), v.y * sin(distance), v.z * sin(distance), cos(distance));
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

	ImGui::SliderFloat("axis change speed 1", &axisChangeSpeed1, 0.0f, 10.0f);
	ImGui::SliderFloat("axis change speed 2", &axisChangeSpeed2, 0.0f, 10.0f);
	ImGui::SliderFloat("rotation speed", &rotationSpeed, 0.0f, 10.0f);

	static f32 elapsed = 0.0f;
	elapsed += Constants::dt;
	/*positions.push_back(positionOnSphere);
	positions.push_back(Vec3(positionOn3Sphere.x, positionOn3Sphere.y, positionOn3Sphere.z));*/

	const auto rotationAxis = cross(positionOnSphere, movementDirection);
	const auto rotation = Quat(Constants::dt * axisChangeSpeed1, rotationAxis);
	positionOnSphere *= rotation;
	//movementDirection *= rotation;

	positionOnSphere = positionOnSphere.normalized();
	movementDirection = cross(rotationAxis, positionOnSphere).normalized();

	//const auto rotationSpeed = 5.0f;
	movementDirection *= Quat(noise.value2d(Vec2(0.0f, elapsed)) * Constants::dt * axisChangeSpeed2, positionOnSphere);

	// This is wrong because it doesn't update the movement direction (positionOnSphere).
	// Is it wrong? If you have a 2 sphere then if you represent the direction as a point on a circle then after moving forward the direction stays the same. Would this work alalogously for the 3 sphere and the attached 2 sphere.
	// This is weird, because the positions are represented extrinsically and the tangent space is represented intrincially.
	// What does wrong mean?
	// This is just drawing a curve parametrized by the velocity represented in the tangent space.
	// If you have constant velocity (constant positionOnSphere) then the projection of the curve on the 3 sphere does move on a closed loop so maybe it is correct?
	// The forward direction should be okay, but in 3d the other axes can rotate, which doesn't happen in 2d. So is this happening?
	// Does doing this create the exponential map coordinates?

	/*
	In general this is just a parametrization of a curve on a n-sphere by velocity.

	If you have a random curve on a n-1 sphere (representing the set of direction of an n manifold) how do you move on the n manifold.
	
	*/

	positionOn3Sphere *= exponentialMap(positionOnSphere * Constants::dt * rotationSpeed);
	//positionOn3Sphere *= exponentialMap(Vec3(1.0f, 0.0f, 0.0f) * Constants::dt * rotationSpeed);
	positionOn3Sphere = positionOn3Sphere.normalized();

	//positionOnSphere.nor
	//const auto movementOn3SphereDirection = ;
	// 
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

 	/*renderer.renderHemispheres();
	renderer.renderCyllinders();
	renderer.coloredShadingModel = Mat4(positionOn3Sphere.toMatrix());*/

	renderer.coloredShadingModel = Mat4(positionOn3Sphere.toMatrix());
	const auto soup = regularPolyhedronPolygonSoup(constView(cubeVertices), constView(cubeFaces), cubeVerticesPerFace);
	const auto dual = dualPolyhedron(soup);
	{
		i32 offset = 0;
		for (i32 faceI = 0; faceI < dual.verticesPerFace.size(); faceI++) {
			const auto verticesInFace = dual.verticesPerFace[faceI];
			for (i32 vertexInFaceI = 0; vertexInFaceI < verticesInFace; vertexInFaceI++) {
				const auto a = positionOn3Sphere * dual.positions[dual.facesVertices[offset + vertexInFaceI]];
				const auto b = positionOn3Sphere * dual.positions[dual.facesVertices[offset + (vertexInFaceI + 1) % verticesInFace]];
				renderer.line(a, b, 0.01, Color3::RED);
				/*dual.positions[dual.facesVertices[offset + vertexInFaceI]];
				renderer.line(Vec3())*/
			}
			offset += dual.verticesPerFace[faceI];
		}
	}
	//renderPolygonSoup(dual);

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
