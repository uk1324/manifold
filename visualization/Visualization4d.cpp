#include "Visualization4d.hpp"
#include <engine/Math/Constants.hpp>
#include <engine/Math/Vec3.hpp>
#include <game/Constants.hpp>
#include <game/MeshUtils.hpp>
#include <engine/Math/Interpolation.hpp>
#include <imgui/imgui.h>
#include <engine/Window.hpp>
#include <glad/glad.h>
#include <engine/Input/Input.hpp>

void Visualization4d::update(Renderer& renderer) {

	if (Input::isKeyDown(KeyCode::X)) {
		Window::close();
	}
	if (Input::isKeyDown(KeyCode::ESCAPE)) {
		Window::toggleCursor();
	}

	const auto size = 100;

	static f32 angle = 0.0f;
	ImGui::SliderAngle("angle", &angle);

	const auto uMin = 0.0f;
	static auto uMax = TAU<f32>;
	const auto vMin = 0.0f;
	static auto vMax = TAU<f32>;
	ImGui::SliderFloat("uMax", &uMax, 0.0, TAU<f32>);
	ImGui::SliderFloat("vMax", &vMax, 0.0, TAU<f32>);
	const auto a = 3.0f;
	const auto b = 4.0f;

	for (i32 vi = 0; vi <= size; vi++) {
		for (i32 ui = 0; ui <= size; ui++) {
			const auto ut = f32(ui) / size;
			const auto vt = f32(vi) / size;
			const auto u = lerp(uMin, uMax, ut);
			const auto v = lerp(vMin, vMax, vt);

			//const auto x = (a + b * cos(v)) * cos(u);
			//const auto y = (a + b * cos(v)) * sin(u);
			//const auto z = b * sin(v) * cos(u / 2.0f);
			//const auto w = b * sin(v) * sin(u / 2.0f);

			const auto R = 1.0f;
			const auto P = 1.0f;
			const auto e = 0.2f;
			const auto x = R * (cos(u / 2.0f) * cos(v) - sin(u / 2.0f) * sin(2.0f * v));
			const auto y = R * (sin(u / 2.0f) * cos(v) - cos(u / 2.0f) * sin(2.0f * v));
			const auto z = P * cos(u) * (1.0f + e * sin(v));
			const auto w = P * sin(u) * (1.0f + e * sin(v));

			const auto m = Vec4(x, y, z, w);

			{
				//const auto a = 3.0f;
				//const auto b = 4.0f;
				//const auto x = (a + b * cos(v)) * cos(u);
				//const auto y = (a + b * cos(v)) * sin(u);
				//const auto z = b * sin(v) * cos(u / 2.0f);
				//const auto w = b * sin(v) * sin(u / 2.0f);
				//const auto q = Quat(angle, Vec3(1.0f).normalized()) * Quat(x, y, z, w);
				//const auto m = Vec4(q.x, q.y, q.z, q.w);


				const auto a = 3.0f;
				const auto b = 4.0f;
				const auto x = R * cos(u);
				const auto y = R * sin(u);
				const auto z = P * cos(v);
				const auto w = P * sin(v);
				const auto q = Quat(angle, Vec3(1.0f).normalized()) * Quat(x, y, z, w);
				const auto m = Vec4(q.x, q.y, q.z, q.w);

				/*renderer.triangles.addVertex(Vertex3Pnt{ .position = m.normalized().xyz(), .normal = Vec3(1.0f), .uv = Vec2(u, v)});*/
				renderer.triangles.addVertex(Vertex3Pnt{ .position = m.xyz(), .normal = Vec3(1.0f), .uv = Vec2(u, v) });
				/*auto m = Vec4(x, y, z, w);
				m = q * Quat(x, y, z, w);*/
				//surface.addVertex(m.normalized().xyz(), n, Vec2(u, v), Vec2(ut, vt));
			}


			//const auto p = Vec3(x, y, z);
			/*const auto p = m.normalized().xyz();
			renderer.triangles.addVertex(Vertex3Pnt{ .position = p, .normal = Vec3(1.0f), .uv = Vec2(u, v) });*/
			/*const auto p = parametrization.position(u, v);
			const auto n = parametrization.normal(u, v);*/
		}
	}


	auto index = [&size](i32 ui, i32 vi) {
		//// Wrap aroud
		//if (ui == size) { ui = 0; }
		//if (vi == size) { vi = 0; }
		return vi * (size + 1) + ui;
	};
	std::vector<i32> indices;
	for (i32 vi = 0; vi < size; vi++) {
		for (i32 ui = 0; ui < size; ui++) {
			const auto i0 = index(ui, vi);
			const auto i1 = index(ui + 1, vi);
			const auto i2 = index(ui + 1, vi + 1);
			const auto i3 = index(ui, vi + 1);
			indicesAddQuad(indices, i0, i1, i2, i3);
			//renderer.triangles.addQuat(i0, i1, i2, i3);
			//indicesAddQuad(surface.indices, i0, i1, i2, i3);
		}
	}

	std::vector<i32> sortedTriangles;
	std::vector<Vec3> triangleCenters;
	for (i32 i = 0; i < indices.size() / 3; i++) {
		Vertex3Pnt v[3];
		getTriangle(renderer.triangles.vertices, indices, v, i);
		triangleCenters.push_back((v[0].position + v[1].position + v[2].position) / 3.0f);
		sortedTriangles.push_back(i);
	}
	const auto cameraPosition = fpsCamera.position;
	std::vector<f32> distances;
	for (i32 i = 0; i < triangleCenters.size(); i++) {
		distances.push_back(triangleCenters[i].distanceSquaredTo(cameraPosition));
	}

	const auto lessThan = [&](i32 a, i32 b) {
		return distances[a] > distances[b];
	};
	std::sort(sortedTriangles.begin(), sortedTriangles.end(), lessThan);

	/*for (i32 i = 0; i < indices.size(); i++) {
		renderer.triangles.indices.push_back(indices[i]);
	}*/
 	for (const auto& triangleIndex : sortedTriangles) {
		const auto i = triangleIndex * 3;
		renderer.triangles.addTri(indices[i], indices[i + 1], indices[i + 2]);
	}

	if (!Window::isCursorEnabled()) {
		fpsCamera.update(Constants::dt);
	}
	const auto view = fpsCamera.viewMatrix();
	const auto aspectRatio = Window::aspectRatio();
	const auto projection = Mat4::perspective(PI<f32> / 2.0f, aspectRatio, 0.1f, 1000.0f);
	const auto swaxpYZ = Mat4(Mat3(Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f), Vec3(0.0f, 1.0f, 0.0f)));
	renderer.transform = projection * view;
	renderer.view = view;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, i32(Window::size().x), i32(Window::size().y));
	renderer.renderTriangles(0.5f);
}
