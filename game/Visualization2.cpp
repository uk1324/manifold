#include "Visualization2.hpp"
#include <engine/Window.hpp>
#include <StructUtils.hpp>
#include <engine/Math/Color.hpp>
#include <engine/Math/Plane.hpp>
#include <engine/Math/Sphere.hpp>
#include <engine/Input/Input.hpp>
#include <imgui/imgui.h>
#include <game/Constants.hpp>
#include <engine/Math/Angles.hpp>

Visualization2 Visualization2::make() {
	auto renderer = GameRenderer::make();

	auto linesVbo = Vbo::generate();
	auto linesIbo = Ibo::generate();
	auto linesVao = createInstancingVao<ColoredShadingShader>(linesVbo, linesIbo, renderer.instancesVbo);

	Window::disableCursor();

	auto r = Visualization2{
		//.crossPolytope = ::crossPolytope(4),
		MOVE(linesVbo),
		MOVE(linesIbo),
		MOVE(linesVao),
		MOVE(renderer),
	};
	{
		const auto c = crossPolytope(4);
		for (i32 i = 0; i < c.vertices.size(); i++) {
			const auto t = f32(i) / f32(c.vertices.size() - 1);
			const auto& vertex = c.vertices[i];
			r.vertices.push_back(Vec4(vertex[0], vertex[1], vertex[2], vertex[3]));
			r.verticesColors.push_back(Color3::fromHsv(t, 1.0f, 1.0f));
		}
		/*for (const auto& vertex : c.vertices) {
		}*/
		for (const auto& edge : c.cells[0]) {
			r.edges.push_back(Edge{ edge[0], edge[1] });
		}
		for (const auto& face : c.cells[1]) {
			r.faces.push_back(Face{ .vertices = std::vector<i32>(face) });
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
		ImGui::GetIO().ConfigFlags &= ~flags;
	} else {
		ImGui::GetIO().ConfigFlags |= flags;
	}
}

#include <game/Polytopes.hpp>
#include <game/Combinatorics.hpp>
#include <iostream>

f32 planeRayIntersection(Vec4 planeNormal, f32 planeD, Vec4 rayOrigin, Vec4 rayDirection) {
	// p = p0 + vt
	// <p, n> = d
	// <p0 + vt, n> = d
	// <p0, n> + t<v, n> = d
	// t<v, n> = d - <p0, n>
	// t = (d - <p0, n>) / <v, n>
	const auto t = (planeD - dot(rayOrigin, planeNormal)) / dot(rayDirection, planeNormal);
	/*if (isinf(t) || isnan(t)) {
		return std::nullopt;
	}*/
	return t;
}

/*
Projects from a point on dot(p, p) = 1 to 3D.
*/

bool isPointAtInfinity(Vec3 v) {
	return isinf(v.x);
}

// https://en.wikipedia.org/wiki/Stereographic_projection
Vec3 stereographicProjection(Vec4 p) {
	const auto a = 1.0f / (1.0f - p.w);
	if (isnan(a) || isinf(a)) {
		return Vec3(INFINITY, INFINITY, INFINITY);
	}
	// p.x, p.y, p.z are all <= 1 so they shouldn't overlflow when multiplied by a finite constant a.
	return Vec3(
		p.x * a,
		p.y * a,
		p.z * a
	);
};
/*
f(x, y, z, w) -> (x / (1 - w), y / (1 - w), z / (1 - w))

The jacobian is
[ f1_x, f1_y, f1_z, f1_w ]
[ f2_x, f2_y, f2_z, f2_w ]
[ f3_x, f3_y, f3_z, f3_w ]

[ 1/(1 - w) 0         0         x/(1-w)^2 ]
[ 0         1/(1 - w) 0         y/(1-w)^2 ]
[ 0         0         1/(1 - w) z/(1-w)^2 ]
*/
Vec3 stereographicProjectionJacobian(Vec4 atPoint, Vec4 onVector) {
	const auto s = 1.0f / (1.0f - atPoint.w);
	const auto s2 = s * s;
	return Vec3(
		s * onVector.x + atPoint.x * s2 * onVector.w,
		s * onVector.y + atPoint.y * s2 * onVector.w,
		s * onVector.z + atPoint.z * s2 * onVector.w
	);
}

Vec4 inverseStereographicProjection(Vec3 p) {
	if (isPointAtInfinity(p)) {
		return Vec4(0.0f, 0.0f, 0.0f, 1.0f);
	}
	const auto s = p.x * p.x + p.y * p.y + p.z * p.z;
	const auto a = 2.0f / (s + 1.0f);
	return Vec4(
		p.x * a,
		p.y * a,
		p.z * a,
		(s - 1.0f) / (s + 1.0f)
	);
}

/*
The geodesics are parts of great circles passing though S3. If you have a point p then the point -p also passes though it.
Any great circle can be written as the intersection dot(p, p) = 1 and dot(n, p) = 0.
dot(-p, -p) = (-1)^2 dot(p, p) = 1
and
dot(n, -p) = 0 = -dot(n, p) = 0
*/
Vec3 antipodalPoint(Vec3 p) {
	if (isPointAtInfinity(p)) {
		return Vec3(0.0f);
	}
	return stereographicProjection(-inverseStereographicProjection(p));
}

f32 det(
	f32 m00, f32 m10,
	f32 m01, f32 m12) {
	return m00 * m12 - m10 * m01;
}

f32 det(
	f32 m00, f32 m10, f32 m20,
	f32 m01, f32 m11, f32 m21,
	f32 m02, f32 m12, f32 m22) {
	return 
		+ m00 * det(m11, m21, m12, m22)
		- m10 * det(m01, m21, m02, m22)
		+ m20 * det(m01, m11, m02, m12);
}
f32 det(Vec3 v0, Vec3 v1, Vec3 v2) {
	return det(
		v0.x, v0.y, v0.z,
		v1.x, v1.y, v1.z,
		v2.x, v2.y, v2.z
	);
}

// Vector perpendicular the plane spanned by v0, v1, v2
Vec4 crossProduct(Vec4 v0, Vec4 v1, Vec4 v2) {
	/*
	| e0   e1   e2   e3   |
	| v0.x v0.y v0.z v0.w | = cross(v0, v1, v2)
	| v1.x v1.y v1.z v1.w |
	| v2.x v2.y v2.z v2.w |
	*/
	return Vec4(
		det(
			v0.y, v0.z, v0.w,
			v1.y, v1.z, v1.w,
			v2.y, v2.z, v2.w
		),
		det(
			v0.x, v0.z, v0.w,
			v1.x, v1.z, v1.w,
			v2.x, v2.z, v2.w
		),
		det(
			v0.x, v0.y, v0.w,
			v1.x, v1.y, v1.w,
			v2.x, v2.y, v2.w
		),
		det(
			v0.x, v0.y, v0.z,
			v1.x, v1.y, v1.z,
			v2.x, v2.y, v2.z
		)
	);
}

void gramSchmidtOrthonormalize(View<Vec4> basis) {
	basis[0] = basis[0].normalized();
	for (i32 i = 1; i < basis.size(); i++) {
		auto& v = basis[i];
		for (i32 j = 0; j < i; j++) {
			v -= dot(v, basis[j]) * basis[j];
		}
		v = v.normalized();
	}
}

Vec3 coordinatesInOrthonormal3Basis(const Vec4 orthonormalBasis[3], Vec4 v) {
	return Vec3(
		dot(v, orthonormalBasis[0]),
		dot(v, orthonormalBasis[1]),
		dot(v, orthonormalBasis[2])
	);
}

Vec4 linearCombination(const Vec4 vs[3], Vec3 v) {
	return vs[0] * v.x + vs[1] * v.y + vs[2] * v.z;
}

std::array<Vec4, 3> orthonormalBasisFor3Space(Vec4 normal) {
	Vec4 candidates[] { 
		Vec4(1.0f, 0.0f, 0.0f, 0.0f), 
		Vec4(0.0f, 1.0f, 0.0f, 0.0f), 
		Vec4(0.0f, 0.0f, 1.0f, 0.0f), 
		Vec4(0.0f, 0.0f, 0.0f, 1.0f) 
	};
	//f32 candidatesLengths[4]{};
	for (i32 i = 0; i < 4; i++) {
		auto& candidate = candidates[i];
		candidate -= dot(candidate, normal) * normal;
		//candidatesLengths[i] = candidate.length();
	}
	// @Performance: Calculate lengths once and sort an array of indices.
	std::ranges::sort(candidates, [](Vec4 a, Vec4 b) -> bool {
		return a.length() < b.length();
	});
	gramSchmidtOrthonormalize(View<Vec4>(candidates, 3));
	return std::array<Vec4, 3>{
		candidates[0],
		candidates[1],
		candidates[2]
	};
}

bool pointsNearlyCoplanar(Vec3 p0, Vec3 p1, Vec3 p2, Vec3 p3, f32 epsilon) {
	// https://math.stackexchange.com/questions/405966/if-i-have-three-points-is-there-an-easy-way-to-tell-if-they-are-collinear
	// https://stackoverflow.com/questions/65396833/testing-three-points-for-collinearity-in-a-numerically-robust-way

	// Using areas or volumes makes it independent of the order of points, but it's probably better to have the biggest or the furthest distance of a point.
	/*f32 basesSquaredAreas[]{
		cross(p0, p1).lengthSquared(),
		cross(p0, p2).lengthSquared(),
		cross(p0, p3).lengthSquared(),

		cross(p1, p2).lengthSquared(),
		cross(p1, p3).lengthSquared(),

		cross(p2, p3).lengthSquared(),
	};*/

	/*const auto p = Plane::fromPoints(p0, p1, p2);
	return abs(p.signedDistance(p3)) <= epsilon;*/
	const auto v0 = p0 - p3;
	const auto v1 = p1 - p3;
	const auto v2 = p2 - p3;
	f32 basesSquaredAreas[]{
		cross(v0, v1).lengthSquared(),
		cross(v0, v2).lengthSquared(),
		cross(v1, v2).lengthSquared(),
	};
	const auto biggestBaseArea = sqrt(std::ranges::max(basesSquaredAreas));
	const auto volume = abs(det(v0, v1, v2));
	const auto parallelepipedHeight = volume / biggestBaseArea;
	return parallelepipedHeight <= epsilon;

	// Assumes that the plane goes though p3
	//const auto v0 = p0 - p3;
	//const auto v1 = p1 - p3;
	//const auto v2 = p2 - p3;
	//const auto volume = abs(det(v0, v1, v2));
	//f32 distances[]{
	//	volume / sqrt(cross(v0, v1).lengthSquared()),
	//	volume / sqrt(cross(v0, v2).lengthSquared()),
	//	volume / sqrt(cross(v1, v2).lengthSquared()),
	//};
	//const auto biggestDistance = std::ranges::max(distances);
	//return biggestDistance <= epsilon;
}

#include <engine/Math/Circle.hpp>

void Visualization2::update() {
	//const auto result = crossPolytope(3);
	togglableCursorUpdate();

	if (Input::isKeyDown(KeyCode::X)) {
		Window::close();
	}
	if (Input::isKeyDown(KeyCode::TAB)) {
		selectedCamera = static_cast<CameraType>((static_cast<int>(selectedCamera) + 1) % 2);
	}

	/*const auto a = nChoosek(5, 2);
	const auto b = nChoosek(12, 2);
	const auto c = nChoosek(12, 7);*/
	//for (i32 dimensionOfPolytope = 2; dimensionOfPolytope < 10; dimensionOfPolytope++) {
	//	const auto polytope = crossPolytope(dimensionOfPolytope);
	//	for (i32 i = 0; i < polytope.cells.size(); i++) {
	//		const auto dimensionOfSimplex = i + 1;
	//		const auto expectedSize = crossPolytopeSimplexCount(dimensionOfPolytope, dimensionOfSimplex);
	//		if (polytope.cells[i].size() != expectedSize) {
	//			ASSERT_NOT_REACHED();
	//		}
	//	}
	//}

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


	const auto width = 0.02f;
	auto stereographicDraw = [&](Vec4 e0, Vec4 e1) {
		const auto p0 = stereographicProjection(e0);
		const auto p1 = stereographicProjection(e1);
		if (isPointAtInfinity(p0) && isPointAtInfinity(p1)) {
			CHECK_NOT_REACHED();
			return;
		}
		if (isPointAtInfinity(p0) || isPointAtInfinity(p1)) {
			auto atInfinity = p0;
			auto finite = p1;
			if (isPointAtInfinity(finite)) {
				std::swap(atInfinity, finite);
			}
			const auto direction = finite.normalized();
			renderer.line(Vec3(0.0f), direction * 1000.0f, width, Color3::WHITE);
			renderer.line(Vec3(0.0f), -direction * 1000.0f, width, Color3::WHITE);
		} else {
			auto p2 = antipodalPoint(p0);
			if (isPointAtInfinity(p2)) {
				p2 = antipodalPoint(p1);
				if (isPointAtInfinity(p2)) {
					CHECK_NOT_REACHED();
					return;
				}
			}

			const auto velocity4 = (e1 - dot(e1, e0.normalized()) * e0.normalized()).normalized();
			const auto velocity3 = stereographicProjectionJacobian(e0, velocity4);

			const auto origin = p0;
			Vec3 v0 = p2 - origin;
			Vec3 v1 = p1 - origin;
			const auto b0 = v0.normalized();
			const auto b1 = (v1 - dot(v1, b0) * b0).normalized();
			const auto t0 = dot(b0, b1);
			auto coordinatesInBasis = [&b0, &b1](Vec3 v) -> Vec2 {
				return Vec2(dot(v, b0), dot(v, b1));
			};
			auto fromCoordinatesInBasis = [&b0, &b1, &origin](Vec2 v) -> Vec3 {
				return v.x * b0 + v.y * b1 + origin;
			};
			const auto c0 = Vec2(0.0f);
			const auto c1 = coordinatesInBasis(v0);
			const auto c2 = coordinatesInBasis(v1);
			const auto circle = Circle::thoughPoints(c0, c1, c2);
			Vec3 center = fromCoordinatesInBasis(circle.center);
			const auto radius = circle.radius;

			//const auto midpoint = ((p2 - center) + (p1 - center)).normalized() + center;

			auto circularDistance = [](Vec3 a, Vec3 b) {
				return acos(std::clamp(dot(a.normalized(), b.normalized()), -1.0f, 1.0f));
			};
			/*
			Wanted to try calculating the arclength by calculating it normally between p0, p1 then checking the the endpoint is correct (by checking the distance of the evavluated curve endpoint to the correct endpoint) and 
			*/

			/*i32 stepCount = 10;
			const auto arclength4 = acos(std::clamp(dot(e0, e1), -1.0f, 1.0f));
			auto previous = e0;
			f32 d = 0.0f;
			for (i32 i = 1; i < stepCount; i++) {
				const auto t = f32(i) / f32(stepCount - 1);
				const auto a = t * arclength4;
				const auto current = cos(a) * e0 + sin(a) * velocity4;
				d += circularDistance(
					stereographicProjection(previous) - center,
					stereographicProjection(current) - center);
				previous = current;
			}*/
			f32 d = circularDistance(p0 - center, p1 - center);
			{
				const auto p = (p0 - center);
				const auto v = velocity3.normalized() * p.length();
				const auto calculatedEndpoint = center + p * cos(d) + v * sin(d);
				//renderer.sphere(calculatedEndpoint, width * 3, Color3::CYAN);
				const auto correctEndpoint = p1;
				if (calculatedEndpoint.distanceTo(correctEndpoint) > 0.01f) {
					d = TAU<f32> - d;
				}
			}
			// This isn't actually the midpoint so there can the correct between this and p0 and p1 might not be the shortest.

			//const auto midpoint = stereographicProjection((e0 + e1).normalized());
			// 
			/*const auto d = 
				circularDistance(p0 - center, midpoint - center) +
				circularDistance(p1 - center, midpoint - center);*/

			//lineGenerator.addCircularArc(p0 - center, velocity3, center, d, width);
			lineGenerator.addCircularArc(p0 - center, velocity3, center, d, width);
			//lineGenerator.addCircularArc(p0 - center, velocity3, center, TAU<f32>, width);
			const auto t1 = p0.distanceTo(center);
			const auto t2 = p1.distanceTo(center);
			//const auto t3 = midpoint.distanceTo(center);
			/*renderer.sphere(p0, width * 3, Color3::RED);
			renderer.sphere(p1, width * 3, Color3::RED);
			renderer.sphere(midpoint, width * 3, Color3::BLUE);
			renderer.sphere(center, width * 3, Color3::GREEN);
			renderer.line(p0, p0 + velocity3.normalized() * 0.5f, 0.01f, Color3::MAGENTA);*/
			//renderer.line(p0, p1, width, Color3::WHITE);
			/*renderer.sphere(p0, width * 3, Color3::RED);
			renderer.sphere(p1, width * 3, Color3::RED);*/
			//renderer.sphere(midpoint, width * 3, Color3::BLUE);
		}
	};

	auto apply = [](Quat q, Vec4 v) {
		const auto r = q * Quat(v.x, v.y, v.z, v.w);
		return Vec4(r.x, r.y, r.z, r.w);
	};
	//auto t = stereographicCamera.position.inverseIfNormalized();
	auto t = stereographicCamera.position;

	std::vector<Vec4> transformedVertices4;
	for (const auto& vertex : vertices) {
		transformedVertices4.push_back(apply(t, vertex));
	}


	/*for (i32 i = 0; i < vertices.size(); i++) {
		const auto v = stereographicProjection(apply(t, vertices[i]));
		if (isPointAtInfinity(v)) {
			continue;
		}
		renderer.sphere(v, width * 3.0f, verticesColors[i]);
	}*/
	for (const auto& edge : edges) {
		auto e0 = vertices[edge.vertices[0]];
		auto e1 = vertices[edge.vertices[1]];

		auto apply = [](Quat q, Vec4 v) {
			const auto r = q * Quat(v.x, v.y, v.z, v.w);
			return Vec4(r.x, r.y, r.z, r.w);
		};
		//auto t = stereographicCamera.position.inverseIfNormalized();
		auto t = stereographicCamera.position;
		e0 = apply(t, e0);
		e1 = apply(t, e1);
		//auto project = [](Vec4 v) -> Vec3 {
		//	const auto planeNormal = Vec4(0.0f, 0.0f, 0.0f, 2.0f);
		//	const auto planeD = 0.0f;
		//	const auto rayOrigin = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
		//	const auto rayDirection = v - rayOrigin;
		//	const auto intersectionT = planeRayIntersection(planeNormal, planeD, rayOrigin, rayDirection);
		//	const auto intersection = rayOrigin + rayDirection * intersectionT;
		//	return Vec3(intersection.x, intersection.y, intersection.z);
		//	/*return Vec3(
		//		p.x,
		//		p.y,
		//		p.z
		//	);*/

		//	/*const auto p = v.normalized();
		//	return Vec3(
		//		p.x,
		//		p.y,
		//		p.z
		//	);*/
		//	/*return Vec3(
		//		v.x,
		//		v.y,
		//		v.z
		//	);*/
		//	/*return Vec3(
		//		v.x / v.w,
		//		v.y / v.w,
		//		v.z / v.w
		//	);*/
		//};
		//renderer.line(project(e0), project(e1), 0.01f, Color3::GREEN);
		stereographicDraw(e0, e1);
		//break;
	}
	{


		std::vector<SphericalPolygonInstance> instances;
		//const auto& face = faces[1];
		//const auto& p0 = transformedVertices4[face.vertices[0]];
		//const auto& p1 = transformedVertices4[face.vertices[1]];
		//const auto& p2 = transformedVertices4[face.vertices[2]];
		//const auto p3 = -p0;
		/*const auto& p0 = transformedVertices4[face.vertices[0]];
		const auto& p1 = transformedVertices4[face.vertices[1]];
		const auto& p2 = transformedVertices4[face.vertices[2]];*/
		const auto& p0 = apply(t, Vec4(1.0f, 0.0f, 0.0f, 0.0f));
		const auto& p1 = apply(t, Vec4(0.0f, 1.0f, 0.0f, 0.0f));
		const auto& p2 = apply(t, Vec4(0.0f, 0.0f, 1.0f, 0.0f));
		const auto p3 = -p0;
		const auto sp0 = stereographicProjection(p0);
		const auto sp1 = stereographicProjection(p1);
		const auto sp2 = stereographicProjection(p2);
		const auto sp3 = stereographicProjection(p3);

		renderer.sphere(sp0, width * 3.0f, Color3::RED);
		renderer.sphere(sp1, width * 3.0f, Color3::RED);
		renderer.sphere(sp2, width * 3.0f, Color3::RED);
		renderer.sphere(sp3, width * 3.0f, Color3::GREEN);
		//renderer.sphere(sp3, width * 3.0f, Color3::RED);

		const Vec3 points[]{ sp0, sp1, sp2, sp3 };
		std::vector<Vec3> finitePoints;
		for (const auto& point : points) {
			if (!isPointAtInfinity(point)) {
				finitePoints.push_back(point);
			}
		}
		struct PlaneData {
			Plane plane;
			f32 distanceTo4thPoint;
		};
		std::vector<PlaneData> possiblePlanes;
		//if (finitePoints.size() == 2) // add 0 

		Vec4 orthonormalBasisFor3SpaceContainingPolygon[]{
			p0 - p3, p1 - p3, p2 - p3
		};
		gramSchmidtOrthonormalize(::view(orthonormalBasisFor3SpaceContainingPolygon));

		auto planeThoughPoints = [&](Vec4 p0, Vec4 p1) {
			const auto e0 = coordinatesInOrthonormal3Basis(orthonormalBasisFor3SpaceContainingPolygon, p0);
			const auto e1 = coordinatesInOrthonormal3Basis(orthonormalBasisFor3SpaceContainingPolygon, p1);
			const auto plane2Normal = cross(e0, e1);
			const auto normalIn4Space = linearCombination(orthonormalBasisFor3SpaceContainingPolygon, plane2Normal);
			return normalIn4Space;
		};
		const auto n0 = planeThoughPoints(p0, p1);
		const auto n1 = planeThoughPoints(p1, p2);
		const auto n2 = planeThoughPoints(p2, p0);
		static bool test = false;
		ImGui::Checkbox("test", &test);
		if (finitePoints.size() == 4) {
			for (i32 i = 0; i < 4; i++) {
				const auto plane = Plane::fromPoints(
					finitePoints[(i + 1) % 4],
					finitePoints[(i + 2) % 4],
					finitePoints[(i + 3) % 4]
				);
				possiblePlanes.push_back(PlaneData{
					.plane = plane,
					.distanceTo4thPoint = plane.distance(finitePoints[i])
				});
			}
			const auto& bestFitPlane = std::ranges::min_element(
				possiblePlanes, 
				[](const PlaneData& a, const PlaneData& b) -> bool {
					return a.distanceTo4thPoint < b.distanceTo4thPoint;
				}
			);
			if (bestFitPlane->distanceTo4thPoint < 0.01f || test) {
				Vec3 untransformedInfinitePlaneMeshNormal = Vec3(0.0f, 1.0f, 0.0f);
				////const auto wantedPlane = Plane::fromPoints(sp0, sp1, sp2);
				/*const auto wantedPlane = bestFitPlane->plane;*/
				const auto wantedPlane = Plane::fromPoints(sp0, sp1, sp2);
				auto rotationAxis = cross(untransformedInfinitePlaneMeshNormal, wantedPlane.n).normalized();
				auto rotationAngle = acos(std::clamp(dot(wantedPlane.n.normalized(),
					untransformedInfinitePlaneMeshNormal.normalized()), -1.0f, 1.0f));

				{
					const auto t0 = wantedPlane.signedDistance(sp0);
					const auto t1 = wantedPlane.signedDistance(sp1);
					const auto t2 = wantedPlane.signedDistance(sp2);
					const auto t3 = wantedPlane.signedDistance(sp3);
					const auto t4 = wantedPlane.signedDistance(Vec3(0.0f));
					const auto t5 = 0.0f;
				}

				/*auto rotationAxis = Vec3(0.0f, 0.0f, 1.0f);
				auto rotationAngle = PI<f32> / 2.0f; */

				if (std::abs(rotationAngle) < 0.01f) {
					rotationAngle = 0.0f;
					rotationAxis = Vec3(1.0f, 0.0f, 0.0f);
				}
				const auto rotation = Quat(rotationAngle, rotationAxis);
				{
					const auto t0 = rotation * untransformedInfinitePlaneMeshNormal;
					const auto t1 = t0.distanceTo(wantedPlane.n);
					const auto t2 = 0.0f;
				}
				renderer.infinitePlanes.push_back(HomogenousInstance{
					.transform = 
						Mat4::translation(wantedPlane.d * wantedPlane.n) * 
						Mat4(rotation.inverseIfNormalized().toMatrix()),
					.n0 = n0,
					.n1 = n1,
					.n2 = n2
				});
				//renderer.line(sp0, sp0 + wantedPlane.n, 0.1f, Color3::BLUE);
				/*renderer.line(sp0, sp0 + wantedPlane.n, 0.1f, Color3::BLUE);
				renderer.line(sp0, sp0 + untransformedInfinitePlaneMeshNormal, 0.1f, Color3::BLUE);
				renderer.line(sp0, sp0 + rotationAxis, 0.1f, Color3::YELLOW);*/
				//renderer.homogenousTriangleRenderer.addTri()
			} else {
				const auto sphere = Sphere::thoughPoints(sp0, sp1, sp2, sp3);

				/*renderer.sphere(stereographicProjection(p0), 0.03f, Color3::MAGENTA);
				renderer.sphere(stereographicProjection(p1), 0.03f, Color3::MAGENTA);
				renderer.sphere(stereographicProjection(p2), 0.03f, Color3::MAGENTA);
				renderer.sphere(stereographicProjection(p3), 0.03f, Color3::MAGENTA);*/
				{
					const auto t0 = sphere.center.distanceSquaredTo(sp0);
					const auto t1 = sphere.center.distanceSquaredTo(sp1);
					const auto t2 = sphere.center.distanceSquaredTo(sp2);
					const auto t3 = sphere.center.distanceSquaredTo(sp3);
					const auto t4 = 0.0f;
				}
				/*
				Finding if a point on a sphere belongs to a polyhedron.
				I don't think the lines projected liens are geodesics of the projected sphere.

				It is probably simplest to work in R^4 for this.
				An intersection of a 3-plane going though 0 with the 3 sphere is a sphere that has it's center at 0.
				Then on that sphere we have the vertices of a polygon.
				If we consider jsut the 3-plane subspace then we can calculate 2-planes going though the origin and pairs of vertices. To check on which side a point is we just need to calculate the dot product with it's normal. We can extend the normal from the 3 space to the whole 4 space and then it will define a 3-plane that still bounds the polygon. Then to check if a point lies on the polygon we just need to calculate the dot products of the points with the normals of the spheres.

				Could probably find an approximate solution by calculating the distance from the plane spanned by the vectors that are the vertices of the plane (there is probably an alaogous formula as for the distance from a line in 3-space). This will only be approximate because it will be the linear distance and not the spherical distance. This is kind of similar what I did with the endpoints in the 2d stereographic line rendering code.
				*/
				const auto plane3Normal = crossProduct(p0 - p3, p1 - p3, p2 - p3).normalized();
				{
					// These should be 0, because the plane should pass though 0.
					const auto t0 = dot(p0, plane3Normal);
					const auto t1 = dot(p1, plane3Normal);
					const auto t2 = dot(p2, plane3Normal);
					const auto t3 = dot(p3, plane3Normal);
				}
				/*
				It might be possible to just calculate the plane the 2 points and the antipodal point are in and then use that for checking which side is it on. The issue would be how to determine the correct orientation. Then instead of sending the 4d plane though 0. It would sent the plane passing though the projected points. One way to calculate this plane might be to fist calculate the original plane (could be precomputed) and then calculate the 3d plane (using a cross product) and the compare the signs of the 2 planes.
				*/
				instances.push_back(SphericalPolygonInstance{
					.transform = Vec4(sphere.center.x, sphere.center.y, sphere.center.z, sphere.radius),
					.n0 = n0,
					.n1 = n1,
					.n2 = n2
				});
				renderer.sphericalPolygonShader.use();
				shaderSetUniforms(
					renderer.sphericalPolygonShader,
					SphericalPolygonVertUniforms{
						.transform = renderer.transform
					}
				);
				shaderSetUniforms(
					renderer.sphericalPolygonShader,
					SphericalPolygonFragUniforms{
						.cameraPosition = Vec3(0.0f)
					}
				);

				drawInstances(renderer.sphereMesh.vao, renderer.instancesVbo, constView(instances), [&](usize count) {
					glDrawElementsInstanced(GL_TRIANGLES, renderer.sphereMesh.indexCount, GL_UNSIGNED_INT, nullptr, count);
				});
			}
		}

		//{
		//	const auto v0 = p0 - p3;
		//	const auto v1 = p1 - p3;
		//	const auto v2 = p2 - p3;
		//	const auto volume = abs(det(v0, v1, v2));
		//	f32 distances[]{
		//		volume / sqrt(cross(v0, v1).lengthSquared()),
		//		volume / sqrt(cross(v0, v2).lengthSquared()),
		//		volume / sqrt(cross(v1, v2).lengthSquared()),
		//	};
		//	const auto biggestDistance = std::ranges::max(distances);
		//	return biggestDistance <= epsilon;
		//}

		if (isPointAtInfinity(sp0) ||
			isPointAtInfinity(sp1) ||
			isPointAtInfinity(sp2) ||
			isPointAtInfinity(sp3) ||
			pointsNearlyCoplanar(sp0, sp1, sp2, sp3, 0.1f)) {

			//Vec3 untransformedInfinitePlaneMeshNormal = Vec3(0.0f, 1.0f, 0.0f);

			////const auto wantedPlane = Plane::fromPoints(sp0, sp1, sp2);
			//const auto wantedPlane = Plane::fromPoints(Vec3(0.0f), sp1, sp2);
			//auto rotationAxis = cross(untransformedInfinitePlaneMeshNormal, wantedPlane.n).normalized();
			//auto rotationAngle = acos(std::clamp(dot(wantedPlane.n, 
			//	untransformedInfinitePlaneMeshNormal), 0.0f, 1.0f));

			//{
			//	const auto t0 = wantedPlane.signedDistance(sp0);
			//	const auto t1 = wantedPlane.signedDistance(sp1);
			//	const auto t2 = wantedPlane.signedDistance(sp2);
			//	const auto t3 = wantedPlane.signedDistance(sp3);
			//	const auto t4 = 0.0f;
			//}

			///*auto rotationAxis = Vec3(0.0f, 0.0f, 1.0f);
			//auto rotationAngle = PI<f32> / 2.0f; */

			//if (std::abs(rotationAngle) < 0.01f) {
			//	rotationAngle = 0.0f;
			//	rotationAxis = Vec3(1.0f, 0.0f, 0.0f);
			//}
			//renderer.infinitePlanes.push_back(HomogenousInstance{
			//	.transform = 
			//		Mat4::translation(wantedPlane.d * wantedPlane.n) * 
			//		Mat4(Quat(-rotationAngle, rotationAxis).toMatrix())
			//});
			//renderer.homogenousTriangleRenderer.addTri()
		}
	}
	/*for (const auto& cell : .cellsOfDimension(1)) {
		renderer.line(Vec3(0.0f), Vec3(1.0f), 0.01f, Color3::GREEN);
	}*/
	//glEnable(GL_CULL_FACE);
	//lineGenerator.addCircularArc(Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f), 0.01f);
	renderer.coloredShadingTrianglesAddMesh(lineGenerator, Color3::WHITE);
	lineGenerator.reset();
	renderer.renderInfinitePlanes();

	/*const auto r = makeIcosphere(2, 1.0f);
	renderer.coloredShadingTrianglesAddMesh(r.positions, r.normals, r.indices, Color3::GREEN);*/

	renderer.renderColoredShadingTriangles(ColoredShadingInstance{
		.model = Mat4::identity
	});
	//glDisable(GL_CULL_FACE);


	renderer.renderCyllinders();
	renderer.renderHemispheres();
}
