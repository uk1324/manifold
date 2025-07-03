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
#include <StringStream.hpp>
#include <Put.hpp>

// cells are neighbours if they share a vertex.
/*
if bomb count = cell count then just set win on first move.
*/

#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#endif 
#include <iostream>

#include <Timer.hpp>
#include <fstream>

i32 boardCellCount[]{
	144,
	120,
	216,
	600,
	720
};

Minesweeper::Minesweeper()
	: t(Polytope())
	, rng(dev()) {

	auto& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;
	colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
	colors[ImGuiCol_Border] = ImVec4(0.00f, 0.33f, 0.89f, 1.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.54f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.40f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.67f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.33f, 0.89f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.13f, 0.63f, 0.12f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.13f, 0.63f, 0.12f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.13f, 0.63f, 0.12f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.33f, 0.89f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.47f, 0.47f, 0.47f, 0.78f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.47f, 0.47f, 0.47f, 1.00f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.40f);
	colors[ImGuiCol_PopupBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.00f, 0.46f, 1.00f, 0.00f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.46f, 1.00f, 0.77f);
	colors[ImGuiCol_TabSelected] = ImVec4(0.00f, 0.44f, 1.00f, 1.00f);
	colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.00f, 0.46f, 1.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.00f, 0.46f, 1.00f, 0.35f);

	style.WindowBorderSize = 1;
	style.FrameBorderSize = 1;
	style.PopupBorderSize = 0;
	style.FrameRounding = 4;
	style.PopupRounding = 4;
	style.GrabRounding = 3;

	const auto defaultDensity = 0.15f;
	for (i32 i = 0; i < 5; i++) {
		bombCountSettings[i] = floor(boardCellCount[i] * defaultDensity);
	}
	/*bombCountSettings[i32(Board::CELL_24_SNUB)] = floor(144 * defaultDensity);
	bombCountSettings[i32(Board::CELL_120)] = floor(120 * defaultDensity);
	bombCountSettings[i32(Board::SUBDIVIDED_HYPERCUBE)] = floor(216 * defaultDensity);
	bombCountSettings[i32(Board::CELL_600)] = floor(600 * defaultDensity);
	bombCountSettings[i32(Board::CELL_600_RECTIFIED)] = floor(720 * defaultDensity);*/

	//const auto ttt = make120cell();
	//Timer t;
	loadBoard(Board::CELL_120);
	//t.tookSeconds("loading board");

	//const auto p = makeSnub24cell();
	//const auto source = outputPolytope4DataCpp("snub24cell", p);
	//std::ofstreamm o("game/Snub24cell.hpp");
	//o << source;
	//exit(0);

	//const auto p = makeRectified600cell();
	
	return;

}

bool ButtonCenteredOnLine(const char* label, float alignment = 0.5f) {
	ImGuiStyle& style = ImGui::GetStyle();

	float size = ImGui::CalcTextSize(label).x + style.FramePadding.x * 2.0f;
	float avail = ImGui::GetContentRegionAvail().x;

	float off = (avail - size) * alignment;
	if (off > 0.0f)
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

	return ImGui::Button(label);
}

struct GameInput {
	bool leftMouseDown = false;
	bool rightMouseDown = false;
};

void Minesweeper::update(GameRenderer& renderer) {
	/*if (Input::isKeyDown(KeyCode::ESCAPE)) {
		Window::toggleCursor();
	}*/
	if (Input::isKeyDown(KeyCode::M)) {
		if (isMenuOpen) {
			closeMenu();
		} else {
			openMenu();
		}
		//isMenuOpen = !isMenuOpen;
		//Window::toggleCursor();
	}
	{
		const auto cursorEnabled = isMenuOpen;
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
		if (cursorEnabled) {
			menuGui();
		}
	}
	GameInput input;
	const auto updateMovement = !isMenuOpen;
	if (updateMovement) {
		if (Input::isMouseButtonDown(MouseButton::LEFT)) {
			input.leftMouseDown = true;
		}
		if (Input::isMouseButtonDown(MouseButton::RIGHT)) {
			input.rightMouseDown = true;
		}
	}
	/*auto p = Input::cursorPosWindowSpace();*/
	/*#ifdef __EMSCRIPTEN__
	auto p0 = Vec2(Input::virtualCursor.x, Input::virtualCursor.y);
	auto p1 = Vec2(ImGui::GetCursorPos().x, ImGui::GetCursorPos().y);
	ImGui::InputFloat2("aa1", p0.data());
	ImGui::InputFloat2("aa2", p1.data());
	#endif */

	//togglableCursorUpdate();

	#ifndef __EMSCRIPTEN__
	if (Input::isKeyDown(KeyCode::X)) {
		Window::close();
	}
	#endif 


	const auto ray = Ray3(stereographicCamera.pos3d(), stereographicCamera.forward3d());
	{
		auto cameraDt = Constants::dt;
		if (isMenuOpen) {
			cameraDt = 0.0f;
		}
		stereographicCamera.update(cameraDt);
	}

	const auto view = stereographicCamera.viewMatrix();
	const auto cameraPosition = stereographicCamera.pos3d();
	renderer.frameUpdate(view, cameraPosition, stereographicCamera);

	const auto view4 = stereographicCamera.view4();
	const auto frustum = Frustum::fromMatrix(renderer.projection * renderer.view);

	std::vector<Vec4> transformedVertices4;
	for (const auto& vertex : t.vertices) {
		transformedVertices4.push_back(view4 * vertex);
	}

	f32 diameter = 0.0f;
	{
		std::vector<Vec4> v;
		auto& vertices = t.cells[0].vertices;
		for (const auto& vertex : vertices) {
			v.push_back(t.vertices[vertex]);
		}

		for (i32 i = 0; i < v.size(); i++) {
			for (i32 j = i + 1; j < v.size(); j++) {
				const auto d = (v[i] - v[j]).length();
				if (d > diameter) {
					diameter = d;
				}
			}
		}
	}

	// Non regular polytopes have multiple cell sizes, but it looks good as is so I will leave it like that.
	//const auto edgeLength = (t.vertices[t.edges[0].vertices[0]] - t.vertices[t.edges[0].vertices[1]]).length();
	const auto polytopeScale = diameter / 0.756f; 
	stereographicCamera.movementSpeed = polytopeScale * 0.4f;
	const auto textSize = 0.1f * polytopeScale;
	const auto sphereRadius = textSize / 2.5f;
	const auto segmentWidth = 0.005f * polytopeScale;


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
	std::optional<Hit> closestUnrevealedHit;
	std::optional<Hit> closestHit;
	for (CellIndex cellI = 0; cellI < t.cells.size(); cellI++) {
		auto& center = cellCentersTransformed[cellI];
		//renderer.sphere(center, radius, Color3::GREEN);
		const auto i = raySphereIntersection(ray, center, sphereRadius);

		if (!i.has_value()) {
			continue;
		}

		if (!isRevealed[cellI]) {
			if (!closestUnrevealedHit.has_value() || *i < closestUnrevealedHit->t) {
				closestUnrevealedHit = Hit{
					.cellI = cellI,
					.t = *i
				};
			}
		}

		if (!isRevealed[cellI] || (isRevealed[cellI] && neighbouringBombsCount[cellI] != 0)) {
			if (!closestHit.has_value() || *i < closestHit->t) {
				closestHit = Hit{
					.cellI = cellI,
					.t = *i
				};
			}
		}
		
	}
	const auto hoverAnimationTimeToFinish = 0.1f;
	if (closestUnrevealedHit.has_value()) {
		updateConstantSpeedT(cellHoverAnimationT[closestUnrevealedHit->cellI], hoverAnimationTimeToFinish, true);
		//renderer.sphere(ray.at(closestHit->t), 0.01f, Color3::WHITE);
		if (!isMarked[closestUnrevealedHit->cellI] && input.leftMouseDown) {
			if (state == State::BEFORE_FIRST_MOVE) {
				startGame(closestUnrevealedHit->cellI);
			} else {
				reveal(closestUnrevealedHit->cellI);
			}
		}

		if (input.rightMouseDown) {
			if (!isRevealed[closestUnrevealedHit->cellI]) {
				isMarked[closestUnrevealedHit->cellI] = !isMarked[closestUnrevealedHit->cellI];
			}
		}
	}
	if (closestHit.has_value()) {
		if (Input::isMouseButtonDown(MouseButton::MIDDLE)) {
			if (highlightNeighbours.has_value() && *highlightNeighbours == closestHit->cellI) {
				highlightNeighbours = std::nullopt;
			} else {
				highlightNeighbours = closestHit->cellI;
			}
		}
	}


	i32 cellsRevealed = 0;
	for (const auto& revaled : isRevealed) {
		if (revaled) {
			cellsRevealed++;
		}
	}
	//ImGui::Text("%d/%d cells revealed", cellsRevealed, t.cells.size());

	for (CellIndex i = 0; i < t.cells.size(); i++) {
		if (closestUnrevealedHit.has_value() && closestUnrevealedHit->cellI == i) {
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
	std::vector<bool> isHighligtedCellNeighbour;
	isHighligtedCellNeighbour.resize(t.cells.size(), false);
	if (highlightNeighbours.has_value()) {
		for (const auto& neighbour : cellToNeighbours[*highlightNeighbours]) {
			isHighligtedCellNeighbour[neighbour] = true;
		}
	}


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

		const auto highlightedCellNeighbour = isHighligtedCellNeighbour[cellI];
		const auto highlightedCell = highlightNeighbours.has_value() && cellI == *highlightNeighbours;

		auto highlightedColor = [&](Vec3 color) {
			if (highlightedCell) {

				//return Vec3(1.0f, 0.9f, 0.5f);
				return Vec3(0.3f, 1.0f, 0.2f);
			}
			if (highlightedCellNeighbour) {
				return Color3::YELLOW;
			} 
			return color;
		};

		if (isRevealed[cellI]) {
			if (isBomb[cellI]) {
				renderer.sphere(center, sphereRadius, highlightedColor(Vec3(0.05f)));
			} else {
				if (c >= 1) {
					renderer.centertedText(center, textSize, std::to_string(c), highlightedColor(color));
				}
			}
		} else {
			Vec3 color;
			if (isMarked[cellI]) {
				if (highlightedCell) {
					color = Vec3(1.0f, 0.44, 0.0f);
				} else if (highlightedCellNeighbour) {
					color = Vec3(1.0f, 0.24, 0.0f);
				} else {
					color = Color3::RED;
				}
			} else {
				color = highlightedColor(Color3::WHITE);
			}
			color = lerp(color * 0.5f, color, cellHoverAnimationT[cellI]);
			renderer.sphere(center, sphereRadius, color);
		}

	}

	/*static f32 desiredDeviation = 0.007;
	static i32 maxAllowedPointCount = 20;*/
	static f32 desiredDeviation = 0.1;
	static i32 maxAllowedPointCount = 5;
	/*ImGui::SliderFloat("desired deviation", &desiredDeviation, 0.007, 0.1f);
	ImGui::SliderInt("max allowed point count", &maxAllowedPointCount, 5, 20);*/
	auto drawSegment = [&renderer, &frustum, &segmentWidth](Vec4 e0, Vec4 e1) {

		const auto p0 = stereographicProjection(e0);
		const auto p1 = stereographicProjection(e1);

		auto velocityOutOfAToB = [](Vec4 a, Vec4 b) {
			const auto velocity4 = (b - dot(b, a.normalized()) * a.normalized()).normalized();
			const auto velocity3 = stereographicProjectionJacobian(a, velocity4);
			return velocity3;
		};

		auto drawLineWithFrustumCulling = [&renderer, &segmentWidth, &frustum](Vec3 v0, Vec3 v1) {
			const auto box = Box3::containingRoundCappedCyllinder(v0, v1, segmentWidth);
			if (!frustum.intersects(box)) {
				return;
			}
			renderer.line(v0, v1, segmentWidth, Color3::WHITE);
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
				const auto box = Box3::containingCircleArcTube(center, p, v, d, segmentWidth);
				
				if (frustum.intersects(box)) {
					const auto pointCount = circleArcPointCountRequiredToAchiveError(desiredDeviation, radius, d, maxAllowedPointCount);
					if (!pointCount.has_value()) {
						return;
					}
					if (*pointCount == 2) {
						renderer.line(p0, p1, segmentWidth, Color3::WHITE);
					} else {
						renderer.lineGenerator.addCircularArc(p, v, center, d, segmentWidth, *pointCount);
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
	}

	 //should also win if every non bomb cell is revealed
	const auto nonBombCellsCount = t.cells.size() - bombCount;
	if (state == State::GAME_IN_PROGRESS) {
		i32 nonBombCellsUncovered = 0;
		for (CellIndex i = 0; i < t.cells.size(); i++) {
			if (!isBomb[i] && isRevealed[i]) {
				nonBombCellsUncovered++;
			}
		}
		if (nonBombCellsUncovered == nonBombCellsCount) {
			state = State::WON;
			openMenu();
		}
	}

	//ImGui::Text("%d/%d edges drawn", edgesDrawn, t.edges.size());

	renderer.renderHemispheres();
	renderer.renderCyllinders();

	//ImGui::Text("line triangle count %d", renderer.lineGenerator.indices.size() / 3);
	renderer.coloredTrianglesAddMesh(renderer.lineGenerator, Color3::WHITE);
	renderer.lineGenerator.reset();
	renderer.renderColoredTriangles(ColoredInstance{
		.color = Color3::WHITE,
		.model = Mat4::identity,
	});

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	renderer.renderText();
	glDisable(GL_BLEND);

	/*glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);*/
	//renderer.gfx2d.camera.aspectRatio = Window::aspectRatio();
	//renderer.gfx2d.disk(Input::cursorPosClipSpace(), 0.025f, Color3::GREEN);
	//renderer.gfx2d.disk(Vec2(0.0f), 0.025f, Color3::GREEN);
	/*renderer.gfx2d.drawDisks();
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);*/

	auto drawArrow = [](Vec2 pos, f32 size, u32 color) {
		auto& dl = *ImGui::GetForegroundDrawList();
		//f32 angle = 1.13f;
		/*f32 angle = 0.36f;*/
		f32 angle = PI<f32> / 2.0f - 0.36f;
		const auto tip = Vec2(0.0f);
		const auto ear0 = tip + Vec2::oriented(angle + PI<f32> / 2.0f) * size * 15.0f;
		const auto ear1 = Vec2(ear0.x, -ear0.y);
		const auto centerMiddle = tip - Vec2(size * 12.0f, 0.0f);
		const auto width = 1.5f * size;
		const auto e0 = centerMiddle + Vec2(0.0f, width);
		const auto e1 = Vec2(e0.x, -e0.y);
		const auto centerBack = tip - Vec2(size * 20.0f, 0.0f);
		const auto b0 = centerBack + Vec2(0.0f, width);
		const auto b1 = Vec2(b0.x, -b0.y);

		const auto center = (tip + centerBack) / 2.0f;

		Vec2 points[] {
			tip, ear0, e0, b0, b1, e1, ear1, tip, ear0
		};
		const auto m = Mat3x2::rotate(PI<f32> + angle) * Mat3x2::translate(pos);
		//c.y = 0.0f;
		for (auto& p : points) {
			/*p -= c;
			p *= scale;
			p += c;*/
			p *= m;
		}
		auto ps = reinterpret_cast<ImVec2*>(points);
		const auto psCount = std::size(points);
		dl.AddPolyline(ps, psCount, 0xFF000000, ImDrawFlags_None, 1.5f * size);
		dl.AddConcavePolyFilled(ps, psCount - 2, color);
	};	
	if (isMenuOpen) {
		#ifdef __EMSCRIPTEN__	
		Input::virtualCursor.x = std::clamp(Input::virtualCursor.x, 0.0, f64(Window::size().x));
		Input::virtualCursor.y = std::clamp(Input::virtualCursor.y, 0.0, f64(Window::size().y));
		#endif

		const u32 white = 0xFFFFFFFF;
		const u32 black = 0xFF000000;
		const auto pos = Input::cursorPosWindowSpace();
		//drawArrow(pos, 1.0f, black, 1.4f);
		//drawArrow(pos, 1.0f, white, 1.0f);
		drawArrow(pos, 0.6f, white);
		//const auto ear0Center = ear0 + Vec2::oriented()

		//dl.AddTriangleFilled(pos + Vec2()
		/*ImGui::GetForegroundDrawList()->AddCircleFilled(ImVec2(pos.x, pos.y), 5.0f, 0xFF000000);
		ImGui::GetForegroundDrawList()->AddCircleFilled(ImVec2(pos.x, pos.y), 4.0f, 0xFFFFFFFF);*/
	} else {
		auto& dl = *ImGui::GetForegroundDrawList();
		auto io = ImGui::GetIO();
		{
			i32 markedCount = 0;
			i32 revealedCount = 0;
			for (i32 i = 0; i < t.cells.size(); i++) {
				if (isMarked[i]) {
					markedCount++;
				}
				if (isRevealed[i]) {
					revealedCount++;
				}
			}
			if (state != State::LOST && state != State::WON) {
				//StringStream s;
				std::stringstream s;
				put(s, "bombs: %\nmarked: %\nrevealed: %/%", bombCount, markedCount, revealedCount, t.cells.size());
				//put(s, "bombs: %", bombCount);
				auto& font = io.FontDefault;
				//put("%", s.string());
				f32 size = 30.0f;
				const auto addText = [&](f32 x, f32 y, u32 color = 0xF0000000) {
					f32 offset = 5.0f;
					const auto& str = s.str();
					dl.AddText(font, size, ImVec2(x + offset, y + offset), color, str.c_str(), str.c_str() + str.size());
					};
				addText(1, 1);
				addText(-1, -1);
				addText(-1, 1);
				addText(1, -1);
				addText(0, 0, 0xFFFFFFFF);
			}
		}
		{
			const auto pos = ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
			const auto size = std::min(io.DisplaySize.y, io.DisplaySize.x) * 0.03f;
			//const auto color = IM_COL32(0, 0xFF, 0, 0xFF);
			const auto color = IM_COL32(0xFF, 0xFF, 0xFF, 0xFF);

			//dl.AddCircleFilled(pos, size, color);
			const auto scale = 10.0f;
			dl.AddRectFilled(ImVec2(pos.x - size / scale, pos.y - size / 2.0f), ImVec2(pos.x + size / scale, pos.y + size / 2.0f), color);
			dl.AddRectFilled(ImVec2(pos.x - size / 2.0f, pos.y - size / scale), ImVec2(pos.x + size / 2.0f, pos.y + size / scale), color);
		}
		
	}
}

void Minesweeper::menuGui() {
	auto uiTableBegin = [](const char* id) -> bool {
		return ImGui::BeginTable("menu", 2);
	};
	auto uiTableEnd = []() -> void {
		ImGui::EndTable();
	};

	thread_local std::string string;
	auto prependWithHashHash = [](const char* str) -> const char* const {
		const auto offset = string.size();
		string += "##";
		string += str;
		string += '\0';
		return string.data() + offset;
	};

	auto leafNodeBegin = [](const char* name) {
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		/*ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		ImGui::TreeNodeEx(name, flags);*/
		ImGui::Text(name);
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
	};

	auto combo = [&](const char* name, i32* currentItem,  View<const char* const> items) -> bool {
		leafNodeBegin(name);
		return ImGui::Combo(prependWithHashHash(name), currentItem, items.data(), items.size());
	};

	auto sliderFloat = [&](const char* name, f32& value, f32 min, f32 max) {
		leafNodeBegin(name);
		ImGui::SliderFloat(prependWithHashHash(name), &value, min, max);
		value = std::clamp(value, min, max);
	};

	auto sliderInt = [&](const char* name, i32& value, i32 min, i32 max) {
		leafNodeBegin(name);
		ImGui::SliderInt(prependWithHashHash(name), &value, min, max);
		value = std::clamp(value, min, max);
	};
	const auto& io = ImGui::GetIO();
	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	//ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x / 3.0f, 0.0f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x / 2.5f, 0.0f), ImGuiCond_Always);
	auto& style = ImGui::GetStyle();
	//ImGuiStyleVar_WindowTitleAlign
	style.WindowTitleAlign = ImVec2(0.5f, 0.5f);

	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	std::string title;
	switch (state) {
		using enum State;
	case BEFORE_FIRST_MOVE: title = ""; break;
	case GAME_IN_PROGRESS: title = ""; break;
	case LOST: title = "you lose"; break;
	case WON: title = "you win"; break;
	}
	title += "##menu";

	const auto oldMenuOpen = isMenuOpen;
	ImGui::Begin(title.c_str(), &isMenuOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
	if (isMenuOpen != oldMenuOpen) {
		if (isMenuOpen) {
			openMenu();
		} else {
			closeMenu();
		}
	}
	ImGui::PopStyleColor();
	// display cell count maybe.

	/*Tiling tiling(makeSubdiviedHypercube2());
	const auto n = tiling.cellsNeighbouringToCell();
	i32 maxNeighbourCount = 0;
	for (i32 i = 0; i < t.cells.size(); i++) {
		maxNeighbourCount = std::max(i32(n[i].size()), maxNeighbourCount);
	}*/
	i32 maxNeighbourCounts[]{
		38, // CELL_24_SNUB,
		12, // CELL_120,
		26, // SUBDIVIDED_HYPERCUBE,
		56, // CELL_600,
		32, // CELL_600_RECTIFIED,
	};

	if (uiTableBegin("game")) {
		combo("board", reinterpret_cast<int*>(&boardSetting), constView(boardStrings));
		sliderInt("bomb count", bombCountSettings[i32(boardSetting)], 0, boardCellCount[i32(boardSetting)] - maxNeighbourCounts[i32(boardSetting)] - 1);
		ImGui::EndTable();
	}

	if (ImGui::TreeNode("controls")) {
		if (ImGui::BeginTable("menu", 2)) {
			auto addEntry = [](const char* a, const char* b) {
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text(a);
				ImGui::TableSetColumnIndex(1);
				ImGui::Text(b);
				//ImGui::SetNextItemWidth(-FLT_MIN);
			};
			addEntry("movement", "WASD");
			addEntry("move up", "space");
			addEntry("move down", "shift");
			addEntry("reveal", "left click");
			addEntry("mark", "right click");
			addEntry("show neighbours", "middle click");
			addEntry("toggle menu", "M");
			addEntry("enable fullscreen", "F");
			ImGui::EndTable();
		}

		if (uiTableBegin("controls")) {
			sliderFloat("mouse sensitivity", mouseSensitivity, 0.5, 2.0f);
			stereographicCamera.rotationSpeed = 0.1f * mouseSensitivity;

			uiTableEnd();
		}

		ImGui::TreePop();
	}

	if (ButtonCenteredOnLine("new game")) {
		loadBoard(boardSetting);
		state = State::BEFORE_FIRST_MOVE;
		closeMenu();
	}
	ImGui::End();
}

void Minesweeper::openMenu() {
	isMenuOpen = true;
	#ifdef __EMSCRIPTEN__
	auto& displaySize = ImGui::GetIO().DisplaySize;
	//const auto center = Window::size() / 2.0f;
	Input::virtualCursor = Vec2T<f64>(displaySize.x / 2.0f, displaySize.y / 2.0f);
	#else
	Window::toggleCursor();
	#endif 
	/*#ifdef __EMSCRIPTEN__
	#include <emscripten/html5.h>
	#endif 
	Window::enableCursor();*/
}

void Minesweeper::closeMenu() {
	isMenuOpen = false;
	#ifndef __EMSCRIPTEN__
	Window::disableCursor();
	#endif 
}

void Minesweeper::loadBoard(Board board) {
	loadedBoard = board;
	switch (board) {
		using enum Board;
	case CELL_120: loadBoard(make120cell()); break;
	case SUBDIVIDED_HYPERCUBE: loadBoard(makeSubdiviedHypercube2()); break;
	case CELL_600: loadBoard(make600cell()); break;
	case CELL_600_RECTIFIED: loadBoard(makeRectified600cell()); break;
	case CELL_24_SNUB: loadBoard(makeSnub24cell()); break;
	}
}

void Minesweeper::loadBoard(const Polytope& polytope) {
	highlightNeighbours = std::nullopt;
	bombCount = bombCountSettings[i32(loadedBoard)];
	t = Tiling(polytope);
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
	auto& face = t.faces[0];
	Vec4 center(0.0f);
	for (const auto& vertex : face.vertices) {
		center += t.vertices[vertex];
	}
	center /= face.vertices.size();
	center = center.normalized();
	/*const auto a = t.vertices[t.edges[0].vertices[0]];
	const auto b = t.vertices[t.edges[0].vertices[1]];
	const auto target = (a + b).normalized();*/
	moveTo(center);
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
	openMenu();
	for (CellIndex i = 0; i < t.cells.size(); i++) {
		isRevealed[i] = true;
	}
}
/*
First move.

1. Allow bomb on first move.

2. If bomb on first move, move to it some predetermined position. For example to the first cell or if locked to the next one and so on.

3. Regenerate the board untill the cell is empty.

4. Remove the first clicked cell from the bomb generating pool.

5. Move the map, such that the first click contains no bomb. This would require some symmetry in the map so that you actually move pieces to eachother. This might also fail.

Intuitively the 5th option seems most fair to me, because it almost always (expect the rare additional case) actually generates a fully random board.
*/
void Minesweeper::startGame(CellIndex firstUncoveredCell) {
	std::set<CellIndex> cellsToAvoid;
	switch (firstMoveSetting) {
		using enum FirstMoveSetting;
	case FIRST_MOVE_NO_BOMB:
		cellsToAvoid.insert(firstUncoveredCell);
		break;
	case FIRST_MOVE_EMPTY_CELL:
		cellsToAvoid.insert(firstUncoveredCell);
		for (const auto& neighbour : cellToNeighbours[firstUncoveredCell]) {
			cellsToAvoid.insert(neighbour);
		}
		break;
	}

	std::vector<CellIndex> possibleBombLocations;
	for (CellIndex i = 0; i < t.cells.size(); i++) {
		if (!cellsToAvoid.contains(i)) {
			possibleBombLocations.push_back(i);
		}
	}
	std::vector<CellIndex> bombLocations;
	std::sample(
		possibleBombLocations.begin(),
		possibleBombLocations.end(),
		std::back_inserter(bombLocations),
		bombCount,
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

