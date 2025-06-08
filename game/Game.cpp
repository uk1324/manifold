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
#include <game/Stereographic.hpp>
#include <iostream>
#include <game/4d.hpp>
#include <random>
#include <game/Physics/Body.hpp>
#include <game/WindowUtils.hpp>

Game Game::make() {
	Window::disableCursor();

	auto r = Game{
		.t = Tiling(make600cell()),
		.world = World(4),
	};
	r.cellToNeighbouringCells = r.t.cellsNeighbouringToCell();

	r.isCellSet.resize(r.t.cells.size(), false);
	r.isCellSet[0] = true;

	return r;
}

void Game::update(GameRenderer& renderer) {
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

	const auto view4 = stereographicCamera.view4();
	renderer.frameUpdate(view, cameraPosition, stereographicCamera);

	ImGui::Combo("tool", reinterpret_cast<int*>(&tool), "building\0pushing\0");

	ImGui::Checkbox("update game of life", &updateGameOfLife);
	if (ImGui::Button("randomly initialize")) {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::bernoulli_distribution d(0.04f);

		for (i32 i = 0; i < t.cells.size(); i++) {
			isCellSet[i] = d(gen);
		}
	}

	terrainGenerationGui();

	std::vector<Vec4> transformedVertices4;
	for (const auto& vertex : t.vertices) {
		transformedVertices4.push_back(view4 * vertex);
	}

	std::vector<i32> setCells;
	for (i32 cellI = 0; cellI < t.cells.size(); cellI++) {
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
	for (i32 faceI = 0; faceI < t.faces.size(); faceI++) {
		auto& face = t.faces[faceI];
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
	auto renderFace = [&](i32 faceI, const Tiling::Cell& cell) {
		const auto& face = t.faces[faceI];

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
		renderFace(face.faceI, t.cells[face.cellI]);
	}

	const auto ray = Ray3(Vec3(0.0f), Vec3(0.0f, 0.0f, 1.0f));
	struct Hit {
		f32 t;
		FaceCellPair* face;
	};
	std::optional<Hit> hit;
	for (auto& face : exposedFaces) {
		const auto& normals = t.faces[face.faceI].edgeNormals;
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
		isFaceExposed.resize(t.faces.size(), false);
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
		isFaceAddedToCellsBodies.resize(t.faces.size(), false);
		for (const auto& body : cellsBodies) {
			isFaceAddedToCellsBodies[body.faceIndex] = true;
		}

		for (const auto& exposedFace : exposedFaces) {
			if (!isFaceAddedToCellsBodies[exposedFace.faceI]) {
				auto& face = t.faces[exposedFace.faceI];
				auto& cell = t.cells[exposedFace.cellI];

				Vec4 normal(0.0f);
				for (i32 i = 0; i < cell.faces.size(); i++) {
					if (cell.faces[i] == exposedFace.faceI) {
						normal = cell.faceNormals[i];
					}
				}
				auto b = world.createWall(
					t.vertices[face.vertices[0]],
					t.vertices[face.vertices[1]],
					t.vertices[face.vertices[2]],
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
		auto& face = t.faces[hit->face->faceI];
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

	static bool updatePhysics = false;
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
	for (i32 cellI = 0; cellI < t.cells.size(); cellI++) {
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

#include <game/Noise.hpp>

void Game::terrainGenerationGui() {
	if (ImGui::Button("generate")) {
		noiseSeed = rand();
		generateTerrain();
	}
	ImGui::SliderFloat("scale", &noiseScale, 0.01f, 1.0f);
	ImGui::SliderFloat("lacunarity", &noiseLacunarity, 1.0f, 2.0f);
	ImGui::SliderFloat("gain", &noiseGain, 0.1f, 1.0f);

	if (randomizeSeed) ImGui::BeginDisabled();
	ImGui::InputInt("seed", &noiseSeed);
	if (randomizeSeed) ImGui::EndDisabled();

}

void Game::generateTerrain() {
	std::vector<f32> x;
	std::vector<f32> y;
	std::vector<f32> z;
	std::vector<f32> w;
	std::vector<f32> out;
	for (const auto& cell : t.cells) {
		auto& c = cell.centroid;
		x.push_back(c.x * noiseScale);
		y.push_back(c.y * noiseScale);
		z.push_back(c.z * noiseScale);
		w.push_back(c.w * noiseScale);
	}
	out.resize(t.cells.size(), 0.0f);
	generateNoise(out.data(), x.data(), y.data(), z.data(), w.data(), i32(t.cells.size()), noiseSeed, noiseGain, noiseLacunarity);
	for (i32 i = 0; i < t.cells.size(); i++) {
		isCellSet[i] = out[i] >= 0.0f;
	}
}
