#include "Minesweeper.hpp"
#include <imgui/imgui.h>
#include <engine/Math/Interpolation.hpp>
#include <engine/Math/Color.hpp>
#include <game/Constants.hpp>
#include <game/Animation.hpp>
#include <game/WindowUtils.hpp>
#include <engine/Input/Input.hpp>
#include <engine/Window.hpp>
#include <engine/Math/Frustum.hpp>
#include <game/Math.hpp>
#include <game/4d.hpp>
#include <engine/Math/Circle.hpp>
#include <engine/Math/Angles.hpp>

Minesweeper::Minesweeper()
	: t(make120cell())
	, rng(dev()) {
	cellToNeighbours = t.cellsNeighbouringToCell();
	cellHoverAnimationT.resize(t.cells.size(), 0.0f);
	initialize();

	auto moveTo = [&](Vec4 p) {
		Vec4 origin = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
		const auto movement = movementForwardOnSphere(
			origin,
			normalizedDirectionFromAToB(origin, p) * sphereAngularDistance(origin, p)
		);
		stereographicCamera.transformation = movement;
	};
	const auto a = t.vertices[t.edges[0].vertices[0]];
	const auto b = t.vertices[t.edges[0].vertices[1]];
	const auto target = (a + b).normalized();
	moveTo(target);
	//const auto a = stereographicCamera.pos4();
	//const auto b = t.cells[2].centroid;
	//const auto movement = movementForwardOnSphere(
	//	a,
	//	normalizedDirectionFromAToB(a, b) * sphereAngularDistance(a, b)
	//);

	//const auto t = moveForwardOnSphere(a, normalizedDirectionFromAToB(a, b) * sphereAngularDistance(a, b));

	//stereographicCamera.transformation = stereographicCamera.transformation * movement;

	/*const auto p = stereographicCamera.pos4();
	const auto aa = 1;*/
	/*stereographicCamera.transformation = Mat4(
		Vec4(1.0f, 0.0f, 0.0f, 0.0f),
		Vec4(0.0f, 1.0f, 0.0f, 0.0f),
		Vec4(0.0f, 0.0f, 1.0f, 0.0f),
		t.cells[0].centroid
	);*/
}

void Minesweeper::update(GameRenderer& renderer) {
	togglableCursorUpdate();

	if (Input::isKeyDown(KeyCode::X)) {
		Window::close();
	}

	ImGui::Begin("minesweeper");
	if (ImGui::Button("new game")) {
		gameResetButNotStarted = false;
	}
	ImGui::End();

	const auto ray = Ray3(stereographicCamera.pos3d(), stereographicCamera.forward3d());
	stereographicCamera.update(Constants::dt);
	stereographicCamera.movementSpeed = 0.4f;
	const auto view = stereographicCamera.viewMatrix();
	const auto cameraPosition = stereographicCamera.pos3d();
	renderer.frameUpdate(view, cameraPosition, stereographicCamera);

	const auto view4 = stereographicCamera.view4();
	const auto frustum = Frustum::fromMatrix(renderer.projection * renderer.view);

	std::vector<Vec4> transformedVertices4;
	for (const auto& vertex : t.vertices) {
		transformedVertices4.push_back(view4 * vertex);
	}
	
	f32 textSize = 0.1f;

	std::vector<Vec3> cellCentersTransformed;
	for (const auto& cell : t.cells) {
		const auto p4 = view4 * cell.centroid;
		const auto p3 = stereographicProjection(p4);
		cellCentersTransformed.push_back(p3);
		//renderer.sphere(p3, 0.01f, Color3::GREEN);
	}

	struct Hit {
		CellIndex cellI;
		f32 t;
	};
	std::optional<Hit> closestHit;
	for (CellIndex cellI = 0; cellI < t.cells.size(); cellI++) {
		if (isRevealed[cellI]) {
			continue;
		}
		auto& center = cellCentersTransformed[cellI];
		const auto radius = textSize / 2.5f;
		//renderer.sphere(center, radius, Color3::GREEN);
		const auto i = raySphereIntersection(ray, center, radius);

		if (!i.has_value()) {
			continue;
		}
		if (!closestHit.has_value() || *i < closestHit->t) {
			closestHit = Hit{
				.cellI = cellI,
				.t = *i
			};
		}
	}
	const auto hoverAnimationTimeToFinish = 0.1f;
	if (closestHit.has_value()) {
		updateConstantSpeedT(cellHoverAnimationT[closestHit->cellI], hoverAnimationTimeToFinish, true);
		//renderer.sphere(ray.at(closestHit->t), 0.01f, Color3::WHITE);
		if (!isMarked[closestHit->cellI] && Input::isMouseButtonDown(MouseButton::LEFT)) {
			if (state == State::BEFORE_FIRST_MOVE) {
				startGame(closestHit->cellI);
			} else {
				reveal(closestHit->cellI);
			}
		}

		if (Input::isMouseButtonDown(MouseButton::RIGHT)) {
			if (!isRevealed[closestHit->cellI]) {
				isMarked[closestHit->cellI] = !isMarked[closestHit->cellI];
			}
		}
	}

	i32 cellsRevealed = 0;
	for (const auto& revaled : isRevealed) {
		if (revaled) {
			cellsRevealed++;
		}
	}
	// should also win if every non bomb cell is revealed
	ImGui::Text("%d/%d cells revealed", cellsRevealed, t.cells.size());

	for (CellIndex i = 0; i < t.cells.size(); i++) {
		if (closestHit.has_value() && closestHit->cellI == i) {
			continue;
		}
		updateConstantSpeedT(cellHoverAnimationT[i], hoverAnimationTimeToFinish, false);
	}

	std::vector<CellIndex> cellsSortedByDistance;
	for (CellIndex i = 0; i < t.cells.size(); i++) {
		cellsSortedByDistance.push_back(i);
	}
	std::ranges::sort(cellsSortedByDistance, [&](i32 a, i32 b) {
		return cellCentersTransformed[a].z > cellCentersTransformed[b].z;
	});
	for (const auto& cellI : cellsSortedByDistance) {
		const auto& center = cellCentersTransformed[cellI];
		// Because they are sorted every one after will also have negative z
		if (center.z < 0.0f) {
			break;
		}
		if (center.length() > 200.0f) {
			continue;
		}

		const auto c = neighbouringBombsCount[cellI];

		Vec3 colors[]{
			Vec3(1.6f, 0.0f, 96.9f) / 100.0f,
			Vec3(0.4f, 49.4f, 0.0f) / 100.0f,
			Vec3(99.6f, 0.0f, 0.0f) / 100.0f,
			Vec3(0.4f, 0.0f, 51.0f) / 100.0f,
			Vec3(49.8f, 0.4f, 0.8f) / 100.0f,
			Vec3(0.0f, 50.2f, 49.8f) / 100.0f,
			//Vec3(0.0f),
			Color3::MAGENTA * 0.2f,
			Vec3(50.2f) / 100.0f,
		};
		const auto n = std::size(colors);

		Vec3 color = Color3::WHITE;
		if (c >= 1) {
			auto i = c - 1;
			const auto m = i % n;
			const auto k = i / n;
			color = colors[m];
		}

		color = lerp(color, color * 0.5f, cellHoverAnimationT[cellI]);
		const auto sphereRadius = textSize / 2.5f;

		if (isRevealed[cellI]) {
			if (isBomb[cellI]) {
				renderer.sphere(center, sphereRadius, Vec3(0.05f));
			} else {
				if (c >= 1) {
					renderer.centertedText(center, textSize, std::to_string(c), color);
				}
			}
		} else {
			Vec3 color;
			if (isMarked[cellI]) {
				color = Color3::RED;
			} else {
				color = Color3::WHITE;
			}
			color = lerp(color * 0.5f, color, cellHoverAnimationT[cellI]);
			renderer.sphere(center, sphereRadius, color);
		}

	}

	/*static f32 desiredDeviation = 0.007;
	static i32 maxAllowedPointCount = 20;*/
	static f32 desiredDeviation = 0.1;
	static i32 maxAllowedPointCount = 5;
	ImGui::SliderFloat("desired deviation", &desiredDeviation, 0.007, 0.1f);
	ImGui::SliderInt("max allowed point count", &maxAllowedPointCount, 5, 20);
	auto drawSegment = [&renderer, &frustum](Vec4 e0, Vec4 e1) {
		const auto width = 0.005f;

		const auto p0 = stereographicProjection(e0);
		const auto p1 = stereographicProjection(e1);

		auto velocityOutOfAToB = [](Vec4 a, Vec4 b) {
			const auto velocity4 = (b - dot(b, a.normalized()) * a.normalized()).normalized();
			const auto velocity3 = stereographicProjectionJacobian(a, velocity4);
			return velocity3;
		};

		auto drawLineWithFrustumCulling = [&renderer, &width, &frustum](Vec3 v0, Vec3 v1) {
			const auto box = Box3::containingRoundCappedCyllinder(v0, v1, width);
			if (!frustum.intersects(box)) {
				return;
			}
			renderer.line(v0, v1, width, Color3::WHITE);
		};

		if (isPointAtInfinity(p0) && isPointAtInfinity(p1)) {
			// Can't tell if they are connected by a line going though infinity or going though the origin. I suspect that it's probably though infinity, but then there is nothing to draw.
			return;
		} else if (isPointAtInfinity(p0) || isPointAtInfinity(p1)) {
			auto atInfinity = p0;
			auto finite = p1;
			auto atInfinity4 = e0;
			auto finite4 = e1;
			if (isPointAtInfinity(finite)) {
				std::swap(atInfinity, finite);
				std::swap(atInfinity4, finite4);
			}
			// Which direction should infinity be aproached from?
			// I know that the line goes though the origin and the finite point.
			const auto directionFromFiniteToInfinity = velocityOutOfAToB(finite4, atInfinity4).normalized();
			drawLineWithFrustumCulling(finite, finite + directionFromFiniteToInfinity * 1000.0f);
		} else {
			auto p2 = antipodalPoint(p0);
			if (isPointAtInfinity(p2)) {
				p2 = antipodalPoint(p1);
				if (isPointAtInfinity(p2)) {
					// This would probably mean that the 2 points are very near the origin. 
					// Could draw a line.
					CHECK_NOT_REACHED();
					return;
				}
			}
			// Lets assume the 3 points are distinct. That is p0 and p1 are not antipodal. This shouldn't be the case when the points near enough to eachother. For example like in a polytope.

			const auto velocityOutOfP0ToP1 = velocityOutOfAToB(e0, e1);

			const auto coordinateSystemOrigin = p0;
			Vec3 v0 = p2 - coordinateSystemOrigin;
			Vec3 v1 = p1 - coordinateSystemOrigin;
			const auto b0 = v0.normalized();
			const auto b1 = (v1 - dot(v1, b0) * b0).normalized();
			auto coordinatesInBasis = [&b0, &b1](Vec3 v) -> Vec2 {
				return Vec2(dot(v, b0), dot(v, b1));
			};
			auto fromCoordinatesInBasis = [&b0, &b1, &coordinateSystemOrigin](Vec2 v) -> Vec3 {
				return v.x * b0 + v.y * b1 + coordinateSystemOrigin;
			};
			const auto c0 = Vec2(0.0f);
			const auto c1 = coordinatesInBasis(v0);
			const auto c2 = coordinatesInBasis(v1);
			const auto circle = Circle::thoughPoints(c0, c1, c2);
			Vec3 center = fromCoordinatesInBasis(circle.center);
			const auto radius = circle.radius;

			
			auto circularDistance = [](Vec3 a, Vec3 b) {
				return acos(std::clamp(dot(a.normalized(), b.normalized()), -1.0f, 1.0f));
			};
			f32 d = circularDistance(p0 - center, p1 - center);
			const auto p = (p0 - center);
			const auto v = velocityOutOfP0ToP1.normalized() * p.length();
			
			{
				const auto calculatedEndpoint0 = center + p * cos(d) + v * sin(d);
				const auto calculatedEndpoint1 = center + p * cos(TAU<f32> -d) + v * sin(TAU<f32> -d);
				const auto correctEndpoint = p1;
				if (calculatedEndpoint0.distanceTo(correctEndpoint) > calculatedEndpoint1.distanceTo(correctEndpoint)) {
					d = TAU<f32> -d;
				}
			}

			if (abs(d) < 0.01f || abs(d - TAU<f32>) < 0.01f) {
				// If the angle is too small then the above check may not work due to floating point errors. This guess can sometimes be wrong that is when the case abs(d - TAU<f32>) actually happens, but it seems to work fine for now.
				drawLineWithFrustumCulling(p0, p1);
			} else {
				const auto box = Box3::containingCircleArcTube(center, p, v, d, width);
				
				if (frustum.intersects(box)) {
					const auto pointCount = circleArcPointCountRequiredToAchiveError(desiredDeviation, radius, d, maxAllowedPointCount);
					if (!pointCount.has_value()) {
						return;
					}
					if (*pointCount == 2) {
						renderer.line(p0, p1, width, Color3::WHITE);
					} else {
						renderer.lineGenerator.addCircularArc(p, v, center, d, width, *pointCount);
					}
				}
			}
		}
	};

	i32 edgesDrawn = 0;
	for (const auto& edge : t.edges) {
		const auto e0 = transformedVertices4[edge.vertices[0]];
		const auto e1 = transformedVertices4[edge.vertices[1]];
		drawSegment(e0, e1);
		//break;
		//const auto width = 0.005f;

		//const auto e0 = transformedVertices4[edge.vertices[0]];
		//const auto e1 = transformedVertices4[edge.vertices[1]];
		//const auto segment = StereographicSegment::fromEndpoints(e0, e1);
		//Box3 box;
		//switch (segment.type) {
		//	using enum StereographicSegment::Type;
		//case CIRCULAR: {
		//	const auto& s = segment.circular;
		//	box = Box3::containingCircleArcTube(s.center, s.start, s.initialVelocity, s.angle, width);
		//	break;
		//}
		//	
		//case LINE: {
		//	const auto& s = segment.line;
		//	box = Box3::containingRoundCappedCyllinder(s.e[0], s.e[1], width);
		//	break;
		//}

		//}
		//if (frustum.intersects(box)) {
		//	edgesDrawn++;
		//	// Using a line instead of a arc shouldn't matter here. Visually it's already wrong, because it's not using the projected version so making it a bit more wrong shouldn't make too much of a difference.
		//	/*switch (segment.type) {
		//		using enum StereographicSegment::Type;
		//	case CIRCULAR: {
		//		const auto& s = segment.circular;
		//		renderer.line(s.start + s.center, s.sample(s.angle), width, Color3::WHITE);
		//		break;
		//	}

		//	case LINE: {
		//		const auto& s = segment.line;
		//		renderer.line(s.e[0], s.e[1], width, Color3::WHITE);
		//		break;
		//	}
		//	}*/
		//	renderer.stereographicLineSegment(segment, width, false);
		//}
		//break;
		//Box3 box{ .center = segment.circular }
	}
	ImGui::Text("%d/%d edges drawn", edgesDrawn, t.edges.size());

	renderer.renderHemispheres();
	renderer.renderCyllinders();

	ImGui::Text("line triangle count %d", renderer.lineGenerator.indices.size() / 3);
	renderer.coloredShadingTrianglesAddMesh(renderer.lineGenerator, Color3::WHITE);
	renderer.lineGenerator.reset();
	renderer.renderColoredShadingTriangles(ColoredShadingInstance{
		.model = Mat4::identity
	});

	renderer.renderText();

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	renderer.gfx2d.camera.aspectRatio = Window::aspectRatio();
	renderer.gfx2d.disk(Vec2(0.0f), 0.025f, Color3::GREEN);
	renderer.gfx2d.drawDisks();
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}


void Minesweeper::initialize() {
	isBomb.clear();
	isBomb.resize(t.cells.size(), false);

	isRevealed.clear();
	isRevealed.resize(t.cells.size(), false);

	neighbouringBombsCount.resize(t.cells.size());

	isMarked.clear();
	isMarked.resize(t.cells.size(), false);
}

void Minesweeper::reveal(CellIndex cell) {
	if (state == State::LOST || state == State::WON) {
		return;
	}

	if (isRevealed[cell]) {
		return;
	}

	if (isBomb[cell]) {
		gameOver();
		return;
	}

	/*std::vector<bool> visited;
	visited.resize(t.cells.size());*/

	std::vector<i32> toVisit;
	toVisit.push_back(cell);
	while (toVisit.size() > 0) {
		const auto c = toVisit.back();
		toVisit.pop_back();
		isRevealed[c] = true;
		if (neighbouringBombsCount[c] == 0) {
			for (const auto& neighbour : cellToNeighbours[c]) {
				if (isRevealed[neighbour] || isBomb[neighbour]) {
					continue;
				}
				toVisit.push_back(neighbour);
			}
		}
	}
}

void Minesweeper::gameOver() {
	state = State::LOST;
	for (CellIndex i = 0; i < t.cells.size(); i++) {
		isRevealed[i] = true;
	}
}

void Minesweeper::startGame(CellIndex firstUncoveredCell) {
	std::vector<CellIndex> possibleBombLocations;
	for (CellIndex i = 0; i < t.cells.size(); i++) {
		if (i != firstUncoveredCell) {
			possibleBombLocations.push_back(i);
		}
	}
	std::vector<CellIndex> bombLocations;
	std::sample(
		possibleBombLocations.begin(),
		possibleBombLocations.end(),
		std::back_inserter(bombLocations),
		15,
		rng
	);
	for (const auto& cellI : bombLocations) {
		isBomb[cellI] = true;
	}
	for (CellIndex i = 0; i < t.cells.size(); i++) {
		auto& count = neighbouringBombsCount[i];
		count = 0;
		for (const auto& neighbour : cellToNeighbours[i]) {
			if (isBomb[neighbour]) {
				count++;
			}
		}
	}

	reveal(firstUncoveredCell);
	state = State::GAME_IN_PROGRESS;
}
