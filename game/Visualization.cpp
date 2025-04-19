#include "Visualization.hpp"
#include <engine/Math/Color.hpp>
#include <engine/Math/Interpolation.hpp>
#include <game/Constants.hpp>
#include <game/Stereographic.hpp>
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
	
	PolygonSoup sphere;
	{
		// Number of cuts on the edge. So now there will be 2 of the original vertices and 3 in between.
		i32 verticesOnEdge = 4;
		i32 verticesOnEdgeWithoutEndpoints = verticesOnEdge - 2;

		// Add the original vertices at the start of the list
		for (i32 i = 0; i < std::size(icosahedronVertices); i++) {
			sphere.positions.push_back(icosahedronVertices[i]);
		}
		auto getOriginalIcosahedronVertex = [&](i32 i) {
			return i;
		};

		// Add the points on the edges.
		for (i32 i = 0; i < std::size(icosahedronEdges); i += 2) {
			const auto endpoint0Index = icosahedronEdges[i];
			const auto endpoint1Index = icosahedronEdges[i + 1];

			for (i32 i = 1; i <= verticesOnEdgeWithoutEndpoints; i++) {
				const auto t = f32(i) / f32(verticesOnEdge - 1);
				const auto v = lerp(
					icosahedronVertices[endpoint0Index],
					icosahedronVertices[endpoint1Index],
					t
				);
				sphere.positions.push_back(v);
			}
		}

		struct EdgeId {
			i32 startIndex;
			bool reversed;
		};

		auto getEdgeId = [](i32 endpoint0, i32 endpoint1) {
			for (i32 i = 0; i < std::size(icosahedronEdges); i += 2) {
				const auto e0 = icosahedronEdges[i];
				const auto startIndex = i;
				const auto e1 = icosahedronEdges[i + 1];
				if (e0 == endpoint0 && e1 == endpoint1) {
					return EdgeId{ startIndex, false };
				} else if (e0 == endpoint1 && e1 == endpoint0) {
					return EdgeId{ startIndex, true };
				}
			}
			ASSERT_NOT_REACHED();
		};

		auto indexOnEdge = [&](const EdgeId& edgeId, i32 i) -> i32 {
			if (edgeId.reversed) {
				// 0 gets mapped to the last index = verticesOnEdge - 1
				i = verticesOnEdge - i - 1;
			}
			if (i == 0) {
				 return icosahedronEdges[edgeId.startIndex];
			}
			if (i == verticesOnEdge - 1) {
				return icosahedronEdges[edgeId.startIndex + 1];
			}
			const auto edgeVerticesOffset = std::size(icosahedronVertices);
			const auto edgeIndex = edgeId.startIndex / 2;
			return edgeVerticesOffset + 
				edgeIndex * verticesOnEdgeWithoutEndpoints +
				(i - 1);
		};

		std::vector<i32> indices;
		for (i32 faceOffset = 0; faceOffset < std::size(icosahedronFaces); faceOffset += icosahedronVerticesPerFace) {

			const auto faceStartOffset = indices.size();

			const auto v0 = icosahedronFaces[faceOffset];
			const auto v1 = icosahedronFaces[faceOffset + 1];
			const auto v2 = icosahedronFaces[faceOffset + 2];

			const auto bottomEdge = getEdgeId(v0, v1);
			const auto leftEdge = getEdgeId(v0, v2);
			const auto rightEdge = getEdgeId(v1, v2);
			/*    (9)
			      / \
			    (7)-(8)
 			    / \ / \
			  (4)-(5)-(6)
			  / \ / \ / \
			(0)-(1)-(2)-(3)

			//Could explicitly create an array that would store the indices of the vertices in the order like this instead of trying to find the indicies using ifs.
			*/
			std::vector<i32> faceIndices;
			for (i32 i = 0; i < verticesOnEdge; i++) {
				faceIndices.push_back(indexOnEdge(bottomEdge, i));
			}

			i32 insideVerticesOnLayer = verticesOnEdgeWithoutEndpoints - 1;
			for (i32 layer = 1; layer < verticesOnEdge - 1; layer++) {
				const auto leftIndex = indexOnEdge(leftEdge, layer);
				const auto rightIndex = indexOnEdge(rightEdge, layer);
				faceIndices.push_back(leftIndex);
				const auto verticesOnLayer = insideVerticesOnLayer + 2;
				for (i32 i = 1; i <= insideVerticesOnLayer; i++) {
					const auto vertex = lerp(
						sphere.positions[leftIndex],
						sphere.positions[rightIndex],
						f32(i) / f32(verticesOnLayer - 1));
					faceIndices.push_back(sphere.positions.size());
					sphere.positions.push_back(vertex);
				}
				faceIndices.push_back(rightIndex);
				insideVerticesOnLayer -= 1;
			}
			faceIndices.push_back(indexOnEdge(leftEdge, verticesOnEdge - 1));

			i32 bottomLayerOffset = 0;
			for (i32 bottomLayerVertexCount = verticesOnEdge; bottomLayerVertexCount > 1; bottomLayerVertexCount--) {
				const auto topLayerOffset = bottomLayerOffset + bottomLayerVertexCount;
				for (i32 i = 0; i < bottomLayerVertexCount - 1; i++) {
					const auto bottom0 = faceIndices[bottomLayerOffset + i];
					const auto bottom1 = faceIndices[bottomLayerOffset + i + 1];
					const auto top = faceIndices[topLayerOffset + i];
					indicesAddTri(sphere.facesVertices, bottom0, bottom1, top);
					sphere.verticesPerFace.push_back(3);
				}

				const auto topLayerVertices = bottomLayerVertexCount - 1;
 				for (i32 i = 0; i < topLayerVertices - 1; i++) {
					const auto top0 = faceIndices[topLayerOffset + i];
					const auto top1 = faceIndices[topLayerOffset + i + 1];
					const auto bottom = faceIndices[bottomLayerOffset + 1 + i];
					indicesAddTri(sphere.facesVertices, top1, top0, bottom);
					sphere.verticesPerFace.push_back(3);
				}

				bottomLayerOffset += bottomLayerVertexCount;
			}

			//i32 insideVerticesOnLayer = verticesOnEdgeWithoutEndpoints - 1;
			//for (i32 layer = 1; layer < verticesOnEdgeWithoutEndpoints - 1; layer++) {
			//	const auto leftVertex = vertices[indexOnEdge(leftEdge, layer)];
			//	const auto rightVertex = vertices[indexOnEdge(rightEdge, layer)];
			//	const auto verticesOnLayer = insideVerticesOnLayer + 2;
			//	for (i32 i = 1; i <= insideVerticesOnLayer; i++) {
			//		const auto vertex = lerp(leftVertex, rightVertex, f32(i) / f32(insideVerticesOnLayer));
			//		vertices.push_back(vertex);
			//	}
			//	insideVerticesOnLayer -= 1;
			//}

			//auto indexLayer = [&](i32 layerIndex, i32 layerStartOffset, i32 verticesPerLayer, i32 i) -> i32 {
			//	if (i == 0) {
			//		return indexOnEdge(leftEdge, layerIndex);
			//	} 
			//	if (i == verticesPerLayer - 1) {
			//		return indexOnEdge(rightEdge, layerIndex);
			//	}
			//	return faceStartOffset + layerStartOffset + (i - 1);
			//};

			///*i32 layerIndex = 0;
			//i32 bottomLayerStartOffset =;*/

			//for (i32 i = 0; i < verticesOnEdge - 1; i++) {
			//	const auto bottomIndex0 = indexOnEdge(bottomEdge, i);
			//	const auto bottomIndex1 = indexOnEdge(bottomEdge, i + 1);
			//	const auto topIndex = indexLayer(1, 0, verticesOnEdge - 1, i);
			//	indicesAddTri(indices, bottomIndex0, bottomIndex1, topIndex);
			//}
			//i32 bottomLayerIndex = 1;
			//i32 bottomLayerOffset = 0;
			//i32 bottomLayerVertexCount = verticesOnEdge - 1;

			//for (i32 layer = 1; layer < verticesOnEdgeWithoutEndpoints - 1; layer++) {
			//	for (i32 i = 0; i < verticesOnEdge - 1; i++) {
			//		const auto bottomIndex0 = indexLayer(bottomLayerIndex, bottomLayerOffset, bottomLayerVertexCount, i);
			//		const auto bottomIndex1 = indexLayer(bottomLayerIndex, bottomLayerOffset, bottomLayerVertexCount, i + 1);
			//		const auto topIndex = indexLayer(bottomLayerIndex + 1, , verticesOnEdge - 1, i);
			//		indicesAddTri(indices, bottomIndex0, bottomIndex1, topIndex);
			//	}
			//}
		}
	}

	return Visualization{
		.sphereMesh = std::move(sphere),
		MOVE(linesVbo),
		MOVE(linesIbo),
		MOVE(linesVao),
		MOVE(renderer),
	};
}

Quat exponentialMap(Vec3 vectorPart) {
	const auto distance = vectorPart.length();
	const auto v = distance == 0.0f ? Vec3(0.0f) : vectorPart / distance;
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
	renderer.renderColoredShadingTriangles(ColoredShadingInstance{
		.model = Mat4::identity
	});
}

// snap to endpoint of previous line.
void Visualization::sphereDrawing() {
	const Vec3 spherePosition = Vec3(0.0f);
	const f32 sphereRadius = 1.0f;

	ImGui::Begin("settings");

	const auto g = generateTetrahedralDirectSymmetriesQuats();
	ImGui::Combo("symmetry group", reinterpret_cast<int*>(&symmetryGroup), "identity\0tetrahedral\0octahedral\0icosahedral\0");
	ImGui::Checkbox("rotate", &rotateRandomly);
	if (rotateRandomly) {
		randomRotationGenerator.settingsGui();
	}
	ImGui::Checkbox("draw stereographic projection", &drawStereographicProjection);
	ImGui::End();

	if (rotateRandomly) {
		randomRotationGenerator.updateRotation();
	}
	const auto sphereRotation = randomRotationGenerator.getRotation();

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
		const auto newPointObjectSpace = sphereRotation * newPoint;
		if (currentlyDrawnLine->size() == 0 || currentlyDrawnLine->back().distanceTo(newPointObjectSpace) > 0.001f) {
			currentlyDrawnLine->push_back(newPointObjectSpace);
		}
	}

	auto drawLine = [&](const std::vector<Vec3>& line) {
		for (i32 i = 0; i < line.size() - 1; i++) {
			renderer.line(line[i], line[i + 1], 0.01f, Color3::RED);
		}
	};

	const auto identitySymmetryGroup = Quat::identity;
	auto symmetries = constView(identitySymmetryGroup);
	switch (symmetryGroup) {
		using enum SymmetryGroup;
	case IDENTITY:
		break;
	case TETRAHEDRAL:
		symmetries = constView(tetrahedralSymmetries);
		break;
	case OCTAHEDRAL:
		symmetries = constView(octahedralSymmetries);
		break;
	case ICOSAHEDRAL:
		symmetries = constView(icosahedralSymmetries);
		break;
	}

	//generateIcosahedralDirectSymmetriesQuats2();

	lineGenerator.reset();
	for (const auto& line : lines) {
		lineGenerator.addLine(line);
	}
	if (currentlyDrawnLine.has_value()) {
		lineGenerator.addLine(*currentlyDrawnLine);
	}
	{
		std::vector<Vertex3Pnc> vertices;
		for (i32 i = 0; i < lineGenerator.vertexCount(); i++) {
			vertices.push_back(Vertex3Pnc{
				.position = lineGenerator.positions[i],
				.normal = lineGenerator.normals[i],
				.color = Vec4(Color3::GREEN, 1.0f),
			});
		}
		linesVbo.allocateData(constView(vertices));
		linesIbo.allocateData(constView(lineGenerator.indices));
	}
	std::vector<ColoredShadingInstance> instances;
	f32 scales[] = { 1.0f, -1.0f };
	for (const auto& scale : scales) {
		for (const auto& isometry : symmetries) {
			instances.push_back(ColoredShadingInstance{
				.model = Mat4(scale * sphereRotation.toMatrix() * isometry.toMatrix())
			});
		}
	}
	renderer.coloredShadingShader.use();
	shaderSetUniforms(renderer.coloredShadingShader, ColoredShadingVertUniforms{
		.transform = renderer.transform,
		.view = renderer.view,
	});
	//glEnable(GL_CULL_FACE);
	drawInstances(linesVao, renderer.instancesVbo, constView(instances), [&](usize instanceCount) {
		glDrawElementsInstanced(GL_TRIANGLES, lineGenerator.indices.size(), GL_UNSIGNED_INT, nullptr, instanceCount);
	});
	//glDisable(GL_CULL_FACE);

	auto isInf = [](f32 v) {
		return isnan(v) || isinf(v);
	};

	if (drawStereographicProjection) {
		lineGenerator.reset();
		for (const auto& isometry : symmetries) {
			for (const auto& line : lines) {
				std::vector<Vec3> transformedPoints;
				for (const auto& point : line) {
					transformedPoints.push_back(point * isometry);
				}

				f32 scales[] = { 1.0f, -1.0f };
				for (const auto& scale : scales) {
					std::vector<Vec3> currentLine;
					for (const auto& line : lines) {
						for (const auto& point : transformedPoints) {
							const auto stereographic = toStereographic(sphereRotation * point * scale);
							auto isInf = [](f32 v) {
								return isnan(v) || isinf(v);
							};

							if (isInf(stereographic.x) || isInf(stereographic.y)) {
								//lineGenerator.addLine(currentLine);
								lineGenerator.addFlatCurve(currentLine, Vec3(0.0f, 1.0f, 0.0f));
								currentLine.clear();
								continue;
							}

							const auto stereographic3 = Vec3(stereographic.x, -1.0f, stereographic.y);
							currentLine.push_back(stereographic3);
						}
					}
					//lineGenerator.addLine(currentLine);
					lineGenerator.addFlatCurve(currentLine, Vec3(0.0f, 1.0f, 0.0f));
					currentLine.clear();
				}
			}
		}
	}

	const auto r = flatShadeConvexPolygonSoup(sphereMesh);
	//renderer.coloredShadingTrianglesAddMesh(lineGenerator, Color3::RED);
	renderer.coloredShadingTrianglesAddMesh(r.positions, r.normals, r.indices, Color3::GREEN);
	renderer.renderColoredShadingTriangles(ColoredShadingInstance{
		.model = Mat4::identity
	});


	auto vertices = View<const Vec3>::empty();
	auto edges = View<const i32>::empty();
	switch (symmetryGroup) {
		using enum SymmetryGroup;
	case IDENTITY:
		break;
	case TETRAHEDRAL:
		vertices = constView(tetrahedronVertices);
		edges = constView(tetrahedronEdges);
		break;
	case OCTAHEDRAL:
		vertices = constView(cubeVertices);
		edges = constView(cubeEdges);
		break;
	case ICOSAHEDRAL:
		vertices = constView(icosahedronVertices);
		edges = constView(icosahedronEdges);
		break;
	}
	lineGenerator.reset();
	for (i32 i = 0; i < edges.size(); i += 2) {
		const auto a = sphereRotation * vertices[edges[i]].normalized();
		const auto b = sphereRotation * vertices[edges[i + 1]].normalized();
		/*renderer.sphere(a, 0.05f, Color3::BLUE);
		renderer.sphere(b, 0.05f, Color3::BLUE);*/
		lineGenerator.addCircularArc(a, b, Vec3(0.0f), 0.01f);
	}
	glEnable(GL_CULL_FACE);
	/*renderer.coloredShadingTrianglesAddMesh(lineGenerator, Color3::RED);
	renderer.renderColoredShadingTriangles(ColoredShadingInstance{
		.model = Mat4::identity
	});*/
	glDisable(GL_CULL_FACE);
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

RandomSmoothRotationGenerator::RandomSmoothRotationGenerator() 
	: noise(0) {}

void RandomSmoothRotationGenerator::settingsGui() {
	ImGui::SliderFloat("rotation speed", &rotationSpeed, 0.0f, 10.0f);
	ImGui::SliderFloat("axis change speed", &axisChangeSpeed, 0.0f, 10.0f);
	ImGui::SliderFloat("axis change change speed", &axisChangeChangeSpeed, 0.0f, 10.0f);
}

void RandomSmoothRotationGenerator::updateRotation() {
	static f32 elapsed = 0.0f;
	elapsed += Constants::dt;

	const auto rotationAxis = cross(positionOnSphere, movementDirection);
	const auto rotation = Quat(Constants::dt * axisChangeSpeed, rotationAxis);
	positionOnSphere *= rotation;

	positionOnSphere = positionOnSphere.normalized();
	movementDirection = cross(rotationAxis, positionOnSphere).normalized();

	movementDirection *= Quat(noise.value2d(Vec2(0.0f, elapsed)) * Constants::dt * axisChangeChangeSpeed, positionOnSphere);

	positionOn3Sphere *= exponentialMap(positionOnSphere * Constants::dt * rotationSpeed);
	positionOn3Sphere = positionOn3Sphere.normalized();
}

const Quat& RandomSmoothRotationGenerator::getRotation() const {
	return positionOn3Sphere;
}
