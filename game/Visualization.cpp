#include "Visualization.hpp"
#include <engine/Math/Color.hpp>
#include <game/Constants.hpp>
#include <engine/Math/Sphere.hpp>
#include <engine/Window.hpp>
#include <imgui/imgui.h>
#include <engine/Input/Input.hpp>
#include <engine/Math/Angles.hpp>
#include <StructUtils.hpp>

Visualization Visualization::make() {
	
	auto renderer = GameRenderer::make();

	auto linesVbo = Vbo::generate();
	auto linesIbo = Ibo::generate();
	auto linesVao = createInstancingVao<ColoredShadingShader>(linesVbo, linesIbo, renderer.instancesVbo);

	Window::disableCursor();

	return Visualization{
		.noise = PerlinNoise(0),
		MOVE(linesVbo),
		MOVE(linesIbo),
		MOVE(linesVao),
		MOVE(renderer),
	};
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
	renderingSetup();

	sphereDrawing();

	render();
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

Vec3 derivBezier(const Vec3* P, const float& t)
{
	return -3 * (1 - t) * (1 - t) * P[0] +
		(3 * (1 - t) * (1 - t) - 6 * t * (1 - t)) * P[1] +
		(6 * t * (1 - t) - 3 * t * t) * P[2] +
		3 * t * t * P[3];
}

Vec3 evalBezierCurve(const Vec3* P, const float& t) {
	float b0 = (1 - t) * (1 - t) * (1 - t);
	float b1 = 3 * t * (1 - t) * (1 - t);
	float b2 = 3 * t * t * (1 - t);
	float b3 = t * t * t;

	return P[0] * b0 + P[1] * b1 + P[2] * b2 + P[3] * b3;
}


void Visualization::sphereDrawing() {
	const Vec3 spherePosition = Vec3(0.0f);
	const f32 sphereRadius = 1.0f;


	renderer.sphere(spherePosition, sphereRadius, Color3::WHITE);

	const Ray3 ray(renderer.cameraPosition, renderer.cameraForward);
	const auto intersection = raySphereIntersection(ray, spherePosition, sphereRadius);

	if (intersection.has_value() && Input::isMouseButtonDown(MouseButton::LEFT)) {
		currentlyDrawnLine = std::vector<Vec3>();
	}

	if (Input::isMouseButtonUp(MouseButton::LEFT) || !intersection.has_value()) {
		if (currentlyDrawnLine.has_value() && currentlyDrawnLine->size() >= 2) {
			lines.push_back(std::move(*currentlyDrawnLine));
		}
		currentlyDrawnLine = std::nullopt;
	}

	if (currentlyDrawnLine.has_value() && Input::isMouseButtonHeld(MouseButton::LEFT) && intersection.has_value()) {
		const auto newPoint = ray.at(*intersection);
		if (currentlyDrawnLine->size() == 0 || currentlyDrawnLine->back().distanceTo(newPoint) > 0.05f) {
			currentlyDrawnLine->push_back(newPoint);
		}
	}

	auto drawLine = [&](const std::vector<Vec3>& line) {
		for (i32 i = 0; i < line.size() - 1; i++) {
			renderer.line(line[i], line[i + 1], 0.01f, Color3::RED);
		}
	};

	static const auto cubeIsometries = cubeDirectIsometries();
	auto drawLineTransformed = [&](const std::vector<Vec3>& line) {
		f32 scales[] = {1.0f, -1.0f};
		for (const auto& scale : scales) {
			for (const auto& isometry : cubeIsometries) {
				for (i32 i = 0; i < line.size() - 1; i++) {
					renderer.line(
						isometry * (line[i] * scale), 
						isometry * (line[i + 1] * scale), 
						0.01f, 
						Color3::RED);
				}
			}
		}
	};

	//for (const auto& line : lines) {
	//	drawLineTransformed(line);
	//}
	//if (currentlyDrawnLine.has_value()) {
	//	drawLineTransformed(*currentlyDrawnLine);
	//}

	std::vector<Vertex3Pnc> vertices;
	std::vector<i32> indices;

	auto outputLine = [&](const std::vector<Vec3>& curvePoints){
		if (curvePoints.size() < 4) {
			return;
		}
		const auto offset = vertices.size();

		uint32_t ndivs = 16;
		uint32_t ncurves = 1 + (curvePoints.size() - 4) / 3;
		Vec3 pts[4];
		std::unique_ptr<Vec3[]> P(new Vec3[(ndivs + 1) * ndivs * ncurves + 1]);
		std::unique_ptr<Vec3[]> N(new Vec3[(ndivs + 1) * ndivs * ncurves + 1]);
		std::unique_ptr<Vec2[]> st(new Vec2[(ndivs + 1) * ndivs * ncurves + 1]);
		for (uint32_t i = 0; i < ncurves; ++i) {
			for (uint32_t j = 0; j < ndivs; ++j) {
				pts[0] = curvePoints[i * 3];
				pts[1] = curvePoints[i * 3 + 1];
				pts[2] = curvePoints[i * 3 + 2];
				pts[3] = curvePoints[i * 3 + 3];
				float s = j / (float)ndivs;
				Vec3 pt = evalBezierCurve(pts, s);
				Vec3 tangent = derivBezier(pts, s).normalized();
				bool swap = false;

				uint8_t maxAxis;
				if (std::abs(tangent.x) > std::abs(tangent.y))
					if (std::abs(tangent.x) > std::abs(tangent.z))
						maxAxis = 0;
					else
						maxAxis = 2;
				else if (std::abs(tangent.y) > std::abs(tangent.z))
					maxAxis = 1;
				else
					maxAxis = 2;

				Vec3 up, forward, right;

				switch (maxAxis) {
				case 0:
				case 1:
					up = tangent;
					forward = Vec3(0, 0, 1);
					//right = up.crossProduct(forward);
					right = cross(up, forward);
					//forward = right.crossProduct(up);
					forward = cross(right, up);
					break;
				case 2:
					up = tangent;
					right = Vec3(0, 0, 1);
					//forward = right.crossProduct(up);
					forward = cross(right, up);
					//right = up.crossProduct(forward);
					right = cross(up, forward);
					break;
				default:
					break;
				};
				
				up = up.normalized();
				forward = forward.normalized();
				right = right.normalized();
				float sNormalized = (i * ndivs + j) / float(ndivs * ncurves);
				//float rad = 0.1 * (1 - sNormalized);
				//float rad = 0.1 * (1 - sNormalized);
				//float rad = 0.01f;
				float rad = 0.04f;
				for (uint32_t k = 0; k <= ndivs; ++k) {
					float t = k / (float)ndivs;
					float theta = t * 2 * PI<f32>;
					Vec3 pc(cos(theta) * rad, 0, sin(theta) * rad);
					float x = pc.x * right.x + pc.y * up.x + pc.z * forward.x;
					float y = pc.x * right.y + pc.y * up.y + pc.z * forward.y;
					float z = pc.x * right.z + pc.y * up.z + pc.z * forward.z;
					P[i * (ndivs + 1) * ndivs + j * (ndivs + 1) + k] = Vec3(pt.x + x, pt.y + y, pt.z + z);
					N[i * (ndivs + 1) * ndivs + j * (ndivs + 1) + k] = Vec3(x, y, z).normalized();
					st[i * (ndivs + 1) * ndivs + j * (ndivs + 1) + k] = Vec2(sNormalized, t);
				}
			}
		}
		P[(ndivs + 1) * ndivs * ncurves] = curvePoints[curvePoints.size() - 1];
		N[(ndivs + 1) * ndivs * ncurves] = (curvePoints[curvePoints.size() - 2] - curvePoints[curvePoints.size() - 1]).normalized();
		st[(ndivs + 1) * ndivs * ncurves] = Vec2(1, 0.5);
		uint32_t numFaces = ndivs * ndivs * ncurves;
		std::unique_ptr<uint32_t[]> verts(new uint32_t[numFaces]);
		for (uint32_t i = 0; i < numFaces; ++i)
			verts[i] = (i < (numFaces - ndivs)) ? 4 : 3;
		std::unique_ptr<uint32_t[]> vertIndices(new uint32_t[ndivs * ndivs * ncurves * 4 + ndivs * 3]);
		uint32_t nf = 0, ix = 0;
		for (uint32_t k = 0; k < ncurves; ++k) {
			for (uint32_t j = 0; j < ndivs; ++j) {
				if (k == (ncurves - 1) && j == (ndivs - 1)) { break; }
				for (uint32_t i = 0; i < ndivs; ++i) {
					indicesAddQuad(
						indices,
						offset + nf,
						offset + nf + (ndivs + 1),
						offset + nf + (ndivs + 1) + 1,
						offset + nf + 1
					);
					/*vertIndices[ix] = nf;
					vertIndices[ix + 1] = nf + (ndivs + 1);
					vertIndices[ix + 2] = nf + (ndivs + 1) + 1;
					vertIndices[ix + 3] = nf + 1;*/
					ix += 4;
					++nf;
				}
				nf++;
			}
		}

		for (uint32_t i = 0; i < ndivs; ++i) {
			vertIndices[ix] = nf;
			vertIndices[ix + 1] = (ndivs + 1) * ndivs * ncurves;
			vertIndices[ix + 2] = nf + 1;
			ix += 3;
			nf++;
		}

		for (i32 i = 0; i < (ndivs + 1) * ndivs * ncurves + 1; i++) {
			vertices.push_back(Vertex3Pnc{
				.position = P[i],
				.normal = N[i],
				.color = Vec4(Color3::RED, 1.0f),
			});
		}
		/*for (i32 i = 0; i < ndivs * ndivs * ncurves * 4 + ndivs * 3; i++) {
			indices.push_back(offset + vertIndices[i]);
		}*/

		//for (const auto& vertex : )
		/*objects.push_back(std::unique_ptr<TriangleMesh>(new TriangleMesh(Matrix44f::kIdentity, numFaces, verts, vertIndices, P, N, st)));*/
	};

	for (const auto& line : lines) {
		outputLine(line);
	}
	if (currentlyDrawnLine.has_value()) {
		outputLine(*currentlyDrawnLine);
	}


	linesVbo.allocateData(vertices.data(), vertices.size() * sizeof(Vertex3Pnc));
	linesIbo.allocateData(indices.data(), indices.size() * sizeof(i32));

	std::vector<ColoredShadingInstance> instances;

	//f32 scales[] = { 1.0f, -1.0f };
	f32 scales[] = { 1.0f };
	for (const auto& scale : scales) {
		for (const auto& isometry : cubeIsometries) {
			instances.push_back(ColoredShadingInstance{
				.model = Mat4(scale * isometry.toMatrix())
			});
		}
	}
	renderer.coloredShadingShader.use();
	shaderSetUniforms(renderer.coloredShadingShader, ColoredShadingVertUniforms{
		.transform = renderer.transform,
		.view = renderer.view,
	});
	drawInstances(linesVao, renderer.instancesVbo, constView(instances), [&](usize instanceCount) {
		glDrawElementsInstanced(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr, instanceCount);
	});

	/*for (const auto& line : lines) {
		drawLine(line);
	}
	if (currentlyDrawnLine.has_value()) {
		drawLine(*currentlyDrawnLine);
	}*/
}

void Visualization::renderingSetup() {
	camera.update(Constants::dt);
	auto view = camera.viewMatrix();
	Vec3 cameraPosition = camera.position;
	const auto cameraForward = (Vec4(Vec3::FORWARD, 0.0f) * view.inversed()).xyz().normalized();
	const auto aspectRatio = Window::aspectRatio();
	const auto projection = Mat4::perspective(PI<f32> / 2.0f, aspectRatio, 0.1f, 1000.0f);
	renderer.transform = projection * view;
	renderer.view = view;
	renderer.projection = projection;
	renderer.cameraForward = cameraForward;
	renderer.cameraPosition = cameraPosition;

	renderer.resizeBuffers(Vec2T<i32>(Window::size()));
	glViewport(0, 0, i32(Window::size().x), i32(Window::size().y));

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_BLEND);

}

void Visualization::render() {
	renderer.renderHemispheres();
	renderer.renderCyllinders();
}
