#include "Game.hpp"
#include <engine/Window.hpp>
#include <StructUtils.hpp>
#include <engine/Math/Color.hpp>
#include <game/Math.hpp>
#include <engine/Math/Plane.hpp>
#include <engine/Math/Sphere.hpp>
#include <engine/Input/Input.hpp>
#include <imgui/imgui.h>
#include <game/Constants.hpp>
#include <engine/Math/Angles.hpp>
#include <game/Polytopes.hpp>
#include <engine/Math/GramSchmidt.hpp>
#include <game/Stereographic.hpp>
#include <iostream>
#include <game/4d.hpp>
#include <game/Physics/Body.hpp>

// https://stackoverflow.com/questions/46770028/is-it-possible-to-use-stdset-intersection-to-check-if-two-sets-have-any-elem
template <class I1, class I2>
bool haveCommonElement(I1 first1, I1 last1, I2 first2, I2 last2) {
	while (first1 != last1 && first2 != last2) {
		if (*first1 < *first2)
			++first1;
		else if (*first2 < *first1)
			++first2;
		else
			return true;
	}
	return false;
}

Game Game::make() {
	auto renderer = GameRenderer::make();

	Window::disableCursor();

	auto r = Game{
		.world = World(4),
		MOVE(renderer),
	};

	for (i32 i = 0; i < 2; i++) {
		r.world.createSphere(randomVec4m1To1().normalized(), 0.1f, 0.5f);
	}

	auto outwardPointingFaceNormal = [&](const std::vector<Vec4>& vertices, const std::vector<Face>& faces, const std::vector<i32>& cellFaces, i32 faceI) {
		const auto& face = faces[faceI];

		auto normal = crossProduct(
			vertices[face.vertices[0]],
			vertices[face.vertices[1]],
			vertices[face.vertices[2]]
		).normalized();
		// The normal should point outward of the cell, that is every vertex of the cell not belonging to the face shuld have a negative dot product with the normal, the code below negates the normal if that is not the case. This is analogous to the case of a sphere. If we have 2 vertices on the sphere we can take their cross product to get the normal of the plane that intersects those 2 vertices. 
		for (const auto& someOtherFaceI : cellFaces) {
			if (someOtherFaceI == faceI) {
				continue;
			}
			const auto& someOtherFace = faces[someOtherFaceI];
			for (const auto& someOtherFaceVertexI : someOtherFace.vertices) {
				bool foundVertexNotBelongingToFace = true;
				for (const auto& faceVertexI : face.vertices) {
					if (someOtherFaceVertexI == faceVertexI) {
						foundVertexNotBelongingToFace = false;
						break;
					}
				}
				if (foundVertexNotBelongingToFace) {
					if (dot(vertices[someOtherFaceVertexI], normal) > 0.0f) {
						normal = -normal;
					}
					return normal;
				}
			}
		}
		CHECK_NOT_REACHED();
		return normal;
	};
	//const auto aa = hypercube(3);

	//const auto c = crossPolytope(4);
	//const auto c = hypercube(4);
	const auto c = subdiviedHypercube4(4);
	//const auto c = make600cell();
	for (i32 i = 0; i < c.vertices.size(); i++) {
		const auto t = f32(i) / f32(c.vertices.size() - 1);
		const auto& vertex = c.vertices[i];
		r.vertices.push_back(Vec4(vertex[0], vertex[1], vertex[2], vertex[3]).normalized());
		r.verticesColors.push_back(Color3::fromHsv(t, 1.0f, 1.0f));
	}
	for (const auto& edge : c.cellsOfDimension(1)) {
		r.edges.push_back(Edge{ edge[0], edge[1] });
	}
	for (const auto& face : c.cellsOfDimension(2)) {
		auto sortedEdges = faceEdgesSorted(c, i32(&face - c.cellsOfDimension(2).data()));
		auto vertices = verticesOfFaceWithSortedEdges(c, sortedEdges);

		Face f{ .vertices = std::move(vertices) };

		Vec4 orthonormalBasisFor3SpaceContainingPolygon[]{
			r.vertices[f.vertices[0]], r.vertices[f.vertices[1]], r.vertices[f.vertices[2]]
		};
		gramSchmidtOrthonormalize(::view(orthonormalBasisFor3SpaceContainingPolygon));

		auto planeThoughPoints = [&](Vec4 p0, Vec4 p1) {
			/*
			If you have a 2-sphere then you can represent lines on it by spheres that intersect the sphere in them. This is the same idea, but we first intersect the 3-sphere with a 3-space to get a sphere and then do the same thing. Then you can check if a point lies on an face using dot products with edges.
			*/
			/*
			Could maybe do this by just using the cross product of e0, e1, and the 3d plane normal, but then I would need to figure out the correct orientation some other way. Thinking about it now I don't really know why it works. That is why the edge normals point in the right direction. It might be accidiental, because the orthonormal basis might not be correctly oriented always.
			*/
			const auto e0 = coordinatesInOrthonormal3Basis(orthonormalBasisFor3SpaceContainingPolygon, p0);
			const auto e1 = coordinatesInOrthonormal3Basis(orthonormalBasisFor3SpaceContainingPolygon, p1);
			const auto plane2Normal = cross(e0, e1);
			const auto normalIn4Space = linearCombination(orthonormalBasisFor3SpaceContainingPolygon, plane2Normal);
			return normalIn4Space;
		};

		i32 previousI = i32(f.vertices.size()) - 1;
		for (i32 i = 0; i < f.vertices.size(); i++) {
			f.edgeNormals.push_back(planeThoughPoints(
				r.vertices[f.vertices[previousI]], 
				r.vertices[f.vertices[i]])
			);
			previousI = i;
		}

		const auto fanBaseVertexI = f.vertices[0];
		const auto& fanBaseVertex = r.vertices[fanBaseVertexI];
		for (i32 i = 1; i < i32(f.vertices.size()) - 1; i++) {
			const auto i0 = f.vertices[i];
			const auto i1 = f.vertices[i + 1];
			Vec4 v0 = r.vertices[i0];
			Vec4 v1 = r.vertices[i1];
			Triangle triangle{
				.vertices = { fanBaseVertexI, i0, i1 },
				.edgeNormals = {
					planeThoughPoints(fanBaseVertex, v0),
					planeThoughPoints(v0, v1),
					planeThoughPoints(v1, fanBaseVertex),
				}
			};
			f.triangulation.push_back(std::move(triangle));
		}

		r.faces.push_back(f);
	}
	for (const auto& cell : c.cellsOfDimension(3)) {
		const auto currentCellI = i32(r.cells.size());

		std::vector<Vec4> faceNormals;
		for (i32 faceI : cell) {
			const auto normal = outwardPointingFaceNormal(r.vertices, r.faces, cell, faceI);
			faceNormals.push_back(normal);
		}

		r.cells.push_back(Cell{ 
			.faces = cell, 
			.faceNormals = std::move(faceNormals),
		});
	}
	r.isCellSet.resize(r.cells.size(), false);
	r.isCellSet[0] = true;
	{
		std::vector<StaticList<i32, 2>> faceToCells;
		faceToCells.resize(r.faces.size());

		for (i32 cellI = 0; cellI < r.cells.size(); cellI++) {
			auto& cell = r.cells[cellI];
			for (const auto& faceI : cell.faces) {
				auto& faceCells = faceToCells[faceI];
				if (faceCells.size() >= 2) {
					ASSERT_NOT_REACHED();
					break;
				}
				faceCells.add(cellI);
			}
		}

		for (i32 faceI = 0; faceI < r.faces.size(); faceI++) {
			auto& cells = faceToCells[faceI];
			ASSERT(cells.size() <= 2);
			for (i32 i = 0; i < 2; i++) {
				r.faces[faceI].cells.add(cells[i]);
				//r.faces[faceI].cells[i] = cells[i];
			}
		}
	}

	for (i32 cellI = 0; cellI < r.cells.size(); cellI++) {
		auto& cell = r.cells[cellI];
		/*for (const auto& faceI : cell.faces) {
			auto& face = r.faces[faceI];
			if (face.cells[0] == cellI) {
				cell.neighbouringCells.push_back(face.cells[1]);
			} else {
				cell.neighbouringCells.push_back(face.cells[0]);
			}
		}*/

		std::set<i32> vertices;
		for (const auto& faceI : cell.faces) {
			for (const auto& vertex : r.faces[faceI].vertices) {
				vertices.insert(vertex);
			}
		}
		r.cellsVertices.push_back(std::move(vertices));
	}

	r.cellToNeighbouringCells.resize(r.cells.size());
	for (i32 cellI = 0; cellI < r.cells.size(); cellI++) {
		for (i32 cellJ = cellI + 1; cellJ < r.cells.size(); cellJ++) {
			auto& vI = r.cellsVertices[cellI];
			auto& vJ = r.cellsVertices[cellJ];
			if (haveCommonElement(vI.begin(), vI.end(), vJ.begin(), vJ.end())) {
				r.cellToNeighbouringCells[cellI].push_back(cellJ);
				r.cellToNeighbouringCells[cellJ].push_back(cellI);
			}
		}
	}

	return r;
}

void togglableCursorUpdate() {
	if (Input::isKeyDown(KeyCode::ESCAPE)) {
		Window::toggleCursor();
	}
	const auto cursorEnabled = Window::isCursorEnabled();
	const auto flags =
		ImGuiConfigFlags_NavNoCaptureKeyboard |
		ImGuiConfigFlags_NoMouse |
		ImGuiConfigFlags_NoMouseCursorChange;

	if (cursorEnabled) {
		Input::ignoreImGuiWantCapture = false;
		ImGui::GetIO().ConfigFlags &= ~flags;
	} else {
		Input::ignoreImGuiWantCapture = true;
		ImGui::GetIO().ConfigFlags |= flags;
	}
}

void Game::update() {
	togglableCursorUpdate();

	if (Input::isKeyDown(KeyCode::X)) {
		Window::close();
	}
	if (Input::isKeyDown(KeyCode::TAB)) {
		selectedCamera = static_cast<CameraType>((static_cast<int>(selectedCamera) + 1) % 2);
	}

	gameOfLifeStep();

	auto view = Mat4::identity;
	Vec3 cameraPosition = Vec3(0.0f);
	switch (selectedCamera) {
		using enum CameraType;
	case NORMAL:
		if (Input::isKeyHeld(KeyCode::LEFT_CONTROL)) {
			stereographicCamera.update(Constants::dt);
		} else {
			camera.update(Constants::dt);
		}
		view = camera.viewMatrix();
		cameraPosition = camera.position;
		break;

	case STEREOGRAPHIC:
		stereographicCamera.update(Constants::dt);
		view = stereographicCamera.viewMatrix();
		cameraPosition = stereographicCamera.pos3d();
		break;
	}
	// Moves the cameraPosition to (0, 0, 0, 1), forward to (0, 0, 1, 0), up to (0, 1, 0, 0) and right to (1, 0, 0, 0). It serves the same purpose as the normal view matrix. That is it moves everyhing to the origin before rendering.
	const auto view4 = stereographicCamera.view4();
	renderer.viewInverse4 = stereographicCamera.view4Inversed();
	renderer.cameraPos4 = stereographicCamera.pos4();

 	const auto cameraForward = (Vec4(Vec3::FORWARD, 0.0f) * view.inversed()).xyz().normalized();
	const auto aspectRatio = Window::aspectRatio();
	const auto projection = Mat4::perspective(PI<f32> / 2.0f, aspectRatio, 0.1f, 1000.0f);
	renderer.transform = projection * view;
	renderer.view = view;
	renderer.projection = projection;
	renderer.cameraForward = cameraForward;
	renderer.cameraPosition = cameraPosition;

	glViewport(0, 0, i32(Window::size().x), i32(Window::size().y));

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_BLEND);

	ImGui::Combo("tool", reinterpret_cast<int*>(&tool), "building\0pushing\0");

	ImGui::Checkbox("update game of life", &updateGameOfLife);
	if (ImGui::Button("randomly initialize")) {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::bernoulli_distribution d(0.04f);

		for (i32 i = 0; i < cells.size(); i++) {
			isCellSet[i] = d(gen);
		}
	}

	std::vector<Vec4> transformedVertices4;
	for (const auto& vertex : vertices) {
		transformedVertices4.push_back(view4 * vertex);
	}

	std::vector<i32> setCells;
	for (i32 cellI = 0; cellI < cells.size(); cellI++) {
		if (isCellSet[cellI]) {
			setCells.push_back(cellI);
		}
	}

	struct FaceCellPair {
		i32 faceI;
		i32 cellI;
		StereographicPlane plane;
	};
	std::vector<FaceCellPair> exposedFaces;
	for (i32 faceI = 0; faceI < faces.size(); faceI++) {
		auto& face = faces[faceI];
		const auto plane = StereographicPlane::fromVertices(
			transformedVertices4[face.vertices[0]],
			transformedVertices4[face.vertices[1]],
			transformedVertices4[face.vertices[2]]
		);
		if (face.cells.size() == 0) {
			CHECK_NOT_REACHED();
			continue;
		}
		if (face.cells.size() == 1) {
			if (isCellSet[face.cells[0]]) {
				exposedFaces.push_back(FaceCellPair{ faceI, face.cells[0], plane });
			}
		} else if (face.cells.size() == 2) {
			const auto& cell0 = face.cells[0];
			const auto& cell1 = face.cells[1];
			if (isCellSet[cell0] && !isCellSet[cell1]) {
				exposedFaces.push_back(FaceCellPair{ faceI, cell0, plane });
			} else if (isCellSet[cell1] && !isCellSet[cell0]) {
				exposedFaces.push_back(FaceCellPair{ faceI, cell1, plane });
			}
		}
	}

	world.settingsGui();

	// @Performance: Frustum culling is probably the simplest optimization to implement to reduce the amount of triangles rendered.
	auto renderFace = [&](i32 faceI, const Cell& cell) {
		const auto& face = faces[faceI];

		// This method renders shader edges for each triangle. Maybe it's possible to do it without that happenning. Maybe pass the adjacent face edge normals. The issue is with which face edges to pick for which triangle.
		/*for (const auto& tri : face.triangulation) {
			const auto& p0 = transformedVertices4[tri.vertices[0]];
			const auto& p1 = transformedVertices4[tri.vertices[1]];
			const auto& p2 = transformedVertices4[tri.vertices[2]];
			Vec4 normal(0.0f);
			for (i32 i = 0; i < cell.faces.size(); i++) {
				if (cell.faces[i] == faceI) {
					normal = cell.faceNormals[i];
				}
			}
			const auto n0 = view4 * tri.edgeNormals[0];
			const auto n1 = view4 * tri.edgeNormals[1];
			const auto n2 = view4 * tri.edgeNormals[2];
			renderer.stereographicTriangle(p0, p1, p2, normal, n0, n1, n2);
		}*/

		const auto& p0 = transformedVertices4[face.vertices[0]];
		const auto& p1 = transformedVertices4[face.vertices[1]];
		const auto& p2 = transformedVertices4[face.vertices[2]];
		Vec4 normal(0.0f);
		for (i32 i = 0; i < cell.faces.size(); i++) {
			if (cell.faces[i] == faceI) {
				normal = cell.faceNormals[i];
			}
		}
		const auto n0 = view4 * face.edgeNormals[0];
		const auto n1 = view4 * face.edgeNormals[1];
		const auto n2 = view4 * face.edgeNormals[2];
		if (face.vertices.size() == 3) {
			renderer.stereographicTriangle(p0, p1, p2, normal, n0, n1, n2, n2, n2);
		} else if (face.vertices.size() == 4) {
			const auto n3 = view4 * face.edgeNormals[3];
			const auto& p3 = transformedVertices4[face.vertices[3]];
			renderer.stereographicTriangle(p0, p1, p2, normal, n0, n1, n2, n3, n3);
			renderer.stereographicTriangle(p0, p2, p3, normal, n0, n1, n2, n3, n3);
		} else if (face.vertices.size() == 5) {
			const auto n3 = view4 * face.edgeNormals[3];
			const auto& p3 = transformedVertices4[face.vertices[3]];
			const auto n4 = view4 * face.edgeNormals[4];
			const auto& p4 = transformedVertices4[face.vertices[4]];
			renderer.stereographicTriangle(p0, p1, p2, normal, n0, n1, n2, n3, n4);
			renderer.stereographicTriangle(p0, p2, p3, normal, n0, n1, n2, n3, n4);
			renderer.stereographicTriangle(p0, p3, p4, normal, n0, n1, n2, n3, n4);
		}
		
		//const auto faceCenter = ((p0 + p1 + p2) / 3.0f).normalized();
		//const auto transformedFaceCenter = stereographicProjection(faceCenter);
		//const auto transformedNormal = stereographicProjectionJacobian(faceCenter, view4 * normal);
		//renderer.sphere(transformedFaceCenter, 0.03f, Color3::RED);
		//renderer.line(transformedFaceCenter, transformedFaceCenter + transformedNormal.normalized(), 0.02f, Color3::MAGENTA);
	};

	//for (const auto& edge : edges) {
	//	renderer.stereographicLineSegment(
	//		transformedVertices4[edge.vertices[0]],
	//		transformedVertices4[edge.vertices[1]]
	//	);
	//}

	for (const auto& face : exposedFaces) {
		renderFace(face.faceI, cells[face.cellI]);
	}

	const auto ray = Ray3(Vec3(0.0f), Vec3(0.0f, 0.0f, 1.0f));
	struct Hit {
		f32 t;
		FaceCellPair* face;
	};
	std::optional<Hit> hit;
	for (auto& face : exposedFaces) {
		const auto& normals = faces[face.faceI].edgeNormals;
		std::vector<Vec4> transformedNormals;
		for (const auto& normal : normals) {
			transformedNormals.push_back(view4 * normal);
		}
		const auto t = rayStereographicPolygonIntersection(ray, face.plane, constView(transformedNormals));
		if (!t.has_value()) {
			continue;
		}
		renderer.sphere(ray.at(*t), 0.01f, Color3::GREEN);
		if (!hit.has_value() || t < hit->t) {
			hit = Hit{ .t = *t, .face = &face };
		}
	}

	if (cellsModified) {
		std::vector<bool> isFaceExposed;
		isFaceExposed.resize(faces.size(), false);
		for (const auto& face : exposedFaces) {
			isFaceExposed[face.faceI] = true;
		}

		cellsBodies.erase(
			std::remove_if(
				cellsBodies.begin(), cellsBodies.end(),
				[&](const CellBody& b) -> bool {
					if (!isFaceExposed[b.faceIndex]) {
						world.bodies.destroy(b.bodyId);
						return true;
					}
					return false;
				}
			),
			cellsBodies.end()
		);

		std::vector<bool> isFaceAddedToCellsBodies;
		isFaceAddedToCellsBodies.resize(faces.size(), false);
		for (const auto& body : cellsBodies) {
			isFaceAddedToCellsBodies[body.faceIndex] = true;
		}

		for (const auto& exposedFace : exposedFaces) {
			if (!isFaceAddedToCellsBodies[exposedFace.faceI]) {
				auto& face = faces[exposedFace.faceI];
				auto& cell = cells[exposedFace.cellI];

				Vec4 normal(0.0f);
				for (i32 i = 0; i < cell.faces.size(); i++) {
					if (cell.faces[i] == exposedFace.faceI) {
						normal = cell.faceNormals[i];
					}
				}
				auto b = world.createWall(
					vertices[face.vertices[0]],
					vertices[face.vertices[1]],
					vertices[face.vertices[2]],
					face.edgeNormals[0],
					face.edgeNormals[1],
					face.edgeNormals[2],
					normal
				);
				cellsBodies.push_back(CellBody{ .faceIndex = exposedFace.faceI, .bodyId = b.id });
			}
		}
	}
	cellsModified = false;


	if (hit.has_value()) {
		auto& face = faces[hit->face->faceI];
		i32 previousI = i32(face.vertices.size()) - 1;
		for (i32 i = 0; i < face.vertices.size(); i++) {
			renderer.stereographicLineSegment(
				transformedVertices4[face.vertices[i]],
				transformedVertices4[face.vertices[previousI]]
			);
			previousI = i;
		}
		renderer.sphere(ray.at(hit->t), 0.01f, Color3::GREEN);

		if (tool == Tool::BUILDING) {
			if (Input::isMouseButtonDown(MouseButton::RIGHT)) {
				if (face.cells[0] != hit->face->cellI) {
					cellsModified = true;
					isCellSet[face.cells[0]] = true;
				} else {
					cellsModified = true;
					isCellSet[face.cells[1]] = true;
				}
			}
			if (Input::isMouseButtonDown(MouseButton::LEFT)) {
				cellsModified = true;
				isCellSet[hit->face->cellI] = false;
			}
		}
	}

	auto drawSphere = [&](Vec4 pos, f32 radius) {
		const auto p4 = view4 * pos;
		const auto s = stereographicSphere(p4, radius);
		const auto transform =
			Mat4::translation(s.center) *
			Mat4(Mat3::scale(s.radius));
		renderer.sphereImpostorCube(pos, transform, s.center, s.radius, Vec4(0.0f), Vec4(0.0f), Vec4(0.0f), Vec4(0.0f), Vec4(0.0f));
	};

	static bool updatePhysics = true;
	if (Input::isKeyDown(KeyCode::H)) {
		updatePhysics = !updatePhysics;
	}

	if (updatePhysics) {
		world.step(1.0f / 60.0f);
	}

	{
		struct BodyHit {
			f32 t;
			Body* b;
		};
		std::optional<BodyHit> bodyHit;
		for (const auto& body : world.bodies) {
			if (body->s) {
				continue;
			}
			drawSphere(body->position, body->radius);
			const auto s = stereographicSphere(view4 * body->position, body->radius);

			const auto hitT = raySphereIntersection(ray, s.center, s.radius);
			if (hitT.has_value() && (!bodyHit.has_value() || *hitT < bodyHit->t)) {
				bodyHit = BodyHit{
					.t = *hitT,
					.b = &body.entity
				};
			}
		}

		if (bodyHit.has_value()) {
			const auto hit = ray.at(bodyHit->t);
			const auto p0 = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
			const auto p1 = inverseStereographicProjection(hit);
			Vec4 direction = p0 - dot(p0, p1) * p1;
			direction = direction.normalized();
			direction *= view4.inversed();
			if (tool == Tool::PUSHING) {
				if (Input::isMouseButtonHeld(MouseButton::LEFT)) {
					/*body->force -= inverseStereographicProjectionJacobian(stereographicProjection(body->position), ray.direction).normalized() * 5.0f;*/
					bodyHit->b->force += direction * 2.0f;
				}
			}
		}
	}

	renderer.renderSphereImpostors();

	renderer.coloredShadingTrianglesAddMesh(renderer.lineGenerator, Color3::WHITE);
	renderer.lineGenerator.reset();
	renderer.renderInfinitePlanes();

	renderer.renderColoredShadingTriangles(ColoredShadingInstance{
		.model = Mat4::identity
	});

	renderer.renderCyllinders();
	renderer.renderHemispheres();

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	renderer.gfx2d.camera.aspectRatio = Window::aspectRatio();
	renderer.gfx2d.disk(Vec2(0.0f), 0.025f, Color3::GREEN);
	renderer.gfx2d.drawDisks();
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

void Game::updateCellsBodies() {
}

void Game::gameOfLifeStep() {
	if (!updateGameOfLife) {
		return;
	}
	frame++;
	if (frame % 30 != 0) {
		return;
	}

	auto oldIsCellSet = isCellSet;
	for (i32 cellI = 0; cellI < cells.size(); cellI++) {
		i32 setNeighbourCount = 0;
		for (const auto& neighbourCellI : cellToNeighbouringCells[cellI]) {
			if (oldIsCellSet[neighbourCellI]) {
				setNeighbourCount++;
			}
		}
		if (const auto isAlive = oldIsCellSet[cellI]) {
			isCellSet[cellI] = setNeighbourCount >= 2 && setNeighbourCount <= 3;
		} else {
			isCellSet[cellI] = setNeighbourCount == 3;
		}
	}
	cellsModified = true;
}
