#include "Visualization2.hpp"
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

Visualization2 Visualization2::make() {
	auto renderer = GameRenderer::make();

	auto linesVbo = Vbo::generate();
	auto linesIbo = Ibo::generate();
	auto linesVao = createInstancingVao<ColoredShadingShader>(linesVbo, linesIbo, renderer.instancesVbo);

	Window::disableCursor();

	auto r = Visualization2{
		MOVE(linesVbo),
		MOVE(linesIbo),
		MOVE(linesVao),
		MOVE(renderer),
	};

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
	const auto c = hypercube(4);
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
		auto vertices = faceVertices(c, face);
		r.faces.push_back(Face{ 
			.vertices = std::move(vertices) 
		});
	}
	for (const auto& cell : c.cellsOfDimension(3)) {
		std::vector<Vec4> faceNormals;
		for (i32 faceI : cell) {
			const auto normal = outwardPointingFaceNormal(r.vertices, r.faces, cell, faceI);
			faceNormals.push_back(normal);
		}
		r.cells.push_back(Cell{ .faces = cell, .faceNormals = std::move(faceNormals) });
	}
	/*r.lodLevelsSettings = {
		{ 0.0f, 5 },
		{ 2.5f, 8 },
		{ 5.0f, 10 },
		{ 10.0f, 15 },
	};*/
	r.lodLevelsSettings = {
		{ 1.0f, 5 },
		{ 5.0f, 8 },
		{ 10.0f, 10 },
		{ 10.0f, 10 },
		{ 40.0f, 15 },
		{ 80.0f, 15 },
	};
	//r.lodLevelsSettings.clear();
	//const f32 radii[]{
	//	1,
	//	5,
	//	10,
	//	20,
	//	40,
	//	80,
	//};
	//for (i32 i = 0; i < std::size(radii); i++) {
	//	const auto radius = radii[i];
	//	const auto unsubdividedEdgeLength = icosahedronVertices[icosahedronEdges[0]].normalized().distanceSquaredTo(icosahedronVertices[icosahedronEdges[1]].normalized());
	//	const auto wantedEdgeSize = 0.2f;
	//	/*
	//	(unsubdividedEdgeLength * radius) / divisionCount < wantedEdgeSize
	//	(unsubdividedEdgeLength * radius) / wantedEdgeSize < divisionCount
	//	*/
	//	const auto divisionCount = i32(std::ceil((unsubdividedEdgeLength * radius) / wantedEdgeSize));
	//	r.lodLevelsSettings.push_back(GameRenderer::SphereLodSetting{
	//		.minRadius = radius,
	//		.divisionCount = divisionCount,
	//	});
	//}


	r.lodLevelsSettings.clear();
	const f32 radii[]{
		1,
		2,
		3,
		5,
		10,
		20,
		40,
		80,
	};
	r.lodLevelsSettings.clear();
	for (i32 i = 0; i < std::size(radii); i++) {
		const auto& radius = radii[i];
		// If I approximate a circle by subdividing it n times then I can calculate the maximum deviation from a cicrcle by calculating (radius - distance of midpoint of segment from center).
		// midpoint 
		//	= (r * (cos(tau/n), sin(tau/n)) + r * (1, 0)) / 2 =
		//	= ((cos(tau/n) + 1, sin(tau/n))) * (r / 2)
		//  length(midpoint) = (r/2) * sqrt((cos(tau/n) + 1)^2 + sin(tau/n)^2) = (*1)
		// (cos(tau/n) + 1)^2 + sin(tau/n)^2 =
		// cos(tau/n)^2 + 2 cos(tau/n) + 1 + sin(tau/n)^2 =
		// 2 cos(tau/n) + 2
		// (*1) = (r/2) * sqrt(2 cos(tau/n) + 2)
		// So the deviation is
		// r - (r/2) * sqrt(2 cos(tau/n) + 2)

		// I want to solve the inequality
		// x in range (0, pi / 2)
		// r - (r/2) * sqrt(2 cos(x) + 2) < a
		//
		// The solution found in desmos is
		// x < arccos(2(a/r - 1)^2 - 1)
		// 
		const auto v0 = icosahedronVertices[icosahedronEdges[0]].normalized();
		const auto v1 = icosahedronVertices[icosahedronEdges[1]].normalized();
		const auto unsubdividedAngleBetweenVertices = v0.shortestAngleTo(v1);
		// x = unsubdividedAngleBetweenVertices / n
		// unsubdividedAngleBetweenVertices / n < arccos(2(a/r - 1)^2 - 1)
		// 1 / n < arccos(2(a/r - 1)^2 - 1) / unsubdividedAngleBetweenVertices
		// n > unsubdividedAngleBetweenVertices / arccos(2(a/r - 1)^2 - 1)
		//const auto desiredDeviation = 0.01f;
		//const auto desiredDeviation = 0.005f;
		const auto desiredDeviation = 0.007f;
		const auto k = acos(2.0f * pow(desiredDeviation / radius - 1.0f, 2.0f) - 1.0f);
		const auto n = i32(std::ceil(unsubdividedAngleBetweenVertices / k));

		// Assuming that the triangles on the sphere are equilateral. You can calculate the maximum deviation from the sphere by calcualting (radius - height of the tetrahedron with vertices being the center of sphere and the vertices of the triangle.
		r.lodLevelsSettings.push_back(GameRenderer::SphereLodSetting{
			.minRadius = radius,
			.divisionCount = n,
		});
	}

	r.renderer.generateSphereLods(r.lodLevelsSettings);
	/*r.renderer.generateSphereLods({
		{ 0.0f, 5 },
		{ 10.0f, 8 },
		{ 20.0f, 10 },
		{ 40.0f, 15 },
	});*/
	//r.stereographicCamera.movementSpeed = 0.2f;
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

void Visualization2::update() {
	lodLevelsSettingsGui();
	//const auto result = crossPolytope(3);
	togglableCursorUpdate();

	if (Input::isKeyDown(KeyCode::X)) {
		Window::close();
	}
	if (Input::isKeyDown(KeyCode::TAB)) {
		selectedCamera = static_cast<CameraType>((static_cast<int>(selectedCamera) + 1) % 2);
	}

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
	auto view4 = stereographicCamera.transformation.inversed();
	renderer.viewInverse4 = stereographicCamera.transformation;
	// This is just extracting the 4th column of the matrix.
	renderer.cameraPos4 = stereographicCamera.transformation * Vec4(0.0f, 0.0f, 0.0f, 1.0f);


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

	static bool drawOnlyLines = false;
	ImGui::Checkbox("drawOnlyLines", &drawOnlyLines);

	const auto width = 0.02f;
	auto stereographicDraw = [&](Vec4 e0, Vec4 e1) {
		const auto segment = StereographicSegment::fromEndpoints(e0, e1);
		switch (segment.type) {
			using enum StereographicSegment::Type;

		case LINE: {
			const auto& p0 = segment.line.e[0];
			const auto& p1 = segment.line.e[1];
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
			}
			break;
		}

		case CIRCULAR: {
			auto& s = segment.circular;
			if (drawOnlyLines) {
				// broken
				renderer.line(s.start, s.sample(s.angle), width, Color3::WHITE);
				break;
			}
			lineGenerator.addCircularArc(s.start, s.initialVelocity, s.center, s.angle, width);
			break;
		}

		}
	};

	std::vector<Vec4> transformedVertices4;
	for (const auto& vertex : vertices) {
		transformedVertices4.push_back(view4 * vertex);
	}


	for (i32 i = 0; i < vertices.size(); i++) {
		const auto v = stereographicProjection(view4 * vertices[i]);
		if (isPointAtInfinity(v)) {
			continue;
		}
		renderer.sphere(v, width * 3.0f, verticesColors[i]);
	}
	for (const auto& edge : edges) {
		auto e0 = vertices[edge.vertices[0]];
		auto e1 = vertices[edge.vertices[1]];

		/*auto apply = [](Quat q, Vec4 v) {
			const auto r = q * Quat(v.x, v.y, v.z, v.w);
			return Vec4(r.x, r.y, r.z, r.w);
		};*/
		//auto t = stereographicCamera.position;
		e0 = view4 * e0;
		e1 = view4 * e1;
		stereographicDraw(e0, e1);
	}

	auto renderPlaneQuad = [&](const Plane& wantedPlane, Vec4 n0, Vec4 n1, Vec4 n2, Vec4 n3, Vec4 planeNormal) {
		Vec3 untransformedPlaneMeshNormal = Vec3(0.0f, 1.0f, 0.0f);
		const auto rotation = unitSphereRotateAToB(untransformedPlaneMeshNormal, wantedPlane.n);

		/*auto rotationAxis = cross(untransformedPlaneMeshNormal, wantedPlane.n).normalized();
		auto rotationAngle = acos(
			std::clamp(
				dot(wantedPlane.n.normalized(), untransformedPlaneMeshNormal.normalized()), 
				-1.0f, 
				1.0f
			)
		);*/

		{
			/*const auto t0 = wantedPlane.signedDistance(sp0);
			const auto t1 = wantedPlane.signedDistance(sp1);
			const auto t2 = wantedPlane.signedDistance(sp2);
			const auto t3 = wantedPlane.signedDistance(sp3);
			const auto t4 = wantedPlane.signedDistance(Vec3(0.0f));
			const auto t5 = 0.0f;*/
		}

		/*auto rotationAxis = Vec3(0.0f, 0.0f, 1.0f);
		auto rotationAngle = PI<f32> / 2.0f; */

		//if (std::abs(rotationAngle) < 0.01f) {
		//	rotationAngle = 0.0f;
		//	rotationAxis = Vec3(1.0f, 0.0f, 0.0f);
		//}
		//const auto rotation = Quat(rotationAngle, rotationAxis);
		//{
		//	const auto t0 = rotation * untransformedPlaneMeshNormal;
		//	const auto t1 = t0.distanceTo(wantedPlane.n);
		//	const auto t2 = 0.0f;
		//}

		renderer.infinitePlanes.push_back(HomogenousInstance{
			.transform =
				Mat4::translation(wantedPlane.d * wantedPlane.n) *
				Mat4(rotation.inverseIfNormalized().toMatrix()),
			.n0 = n0,
			.n1 = n1,
			.n2 = n2,
			.n3 = n3,
			.planeNormal = planeNormal,
		});
	};

	auto renderPlaneTriangle = [&](const Plane& wantedPlane, Vec4 n0, Vec4 n1, Vec4 n2, Vec4 planeNormal) {
		renderPlaneQuad(wantedPlane, n0, n1, n2, n2, planeNormal);
	};

	auto renderSphericalQuad = [&](Vec3 sp0, const Sphere& sphere, Vec4 n0, Vec4 n1, Vec4 n2, Vec4 n3, Vec4 planeNormal) {
		//const auto sphere = Sphere::thoughPoints(sp0, sp1, sp2, sp3);
		// Though if it would be possible to replace the spheres with just their projectsions. That is to render planes instead of spheres. If you did that the circular segments would also need to be replaced with straight lines, because otherwise there would be gaps. If they are replaced then their widht wouldn't change with distance because they wouldn't get further away. You also wouldn't be able to calculate the distance both in 3d and 4d in the shader, because the points are in wrong positions so fading based on distance and shading would be impossible.

		//{
		//	const auto t0 = sphere.center.distanceSquaredTo(sp0);
		//	const auto t1 = sphere.center.distanceSquaredTo(sp1);
		//	const auto t2 = sphere.center.distanceSquaredTo(sp2);
		//	const auto t3 = sphere.center.distanceSquaredTo(sp3);
		//	const auto t4 = 0.0f;
		//}
		/*
		Finding if a point on a sphere belongs to a polyhedron.
		I don't think the lines projected liens are geodesics of the projected sphere.

		It is probably simplest to work in R^4 for this.
		An intersection of a 3-plane going though 0 with the 3 sphere is a sphere that has it's center at 0.
		Then on that sphere we have the vertices of a polygon.
		If we consider jsut the 3-plane subspace then we can calculate 2-planes going though the origin and pairs of vertices. To check on which side a point is we just need to calculate the dot product with it's normal. We can extend the normal from the 3 space to the whole 4 space and then it will define a 3-plane that still bounds the polygon. Then to check if a point lies on the polygon we just need to calculate the dot products of the points with the normals of the spheres.

		Could probably find an approximate solution by calculating the distance from the plane spanned by the vectors that are the vertices of the plane (there is probably an alaogous formula as for the distance from a line in 3-space). This will only be approximate because it will be the linear distance and not the spherical distance. This is kind of similar what I did with the endpoints in the 2d stereographic line rendering code.
		*/
		//const auto plane3Normal = crossProduct(p0 - p3, p1 - p3, p2 - p3).normalized();
		//{
		//	// These should be 0, because the plane should pass though 0.
		//	const auto t0 = dot(p0, plane3Normal);
		//	const auto t1 = dot(p1, plane3Normal);
		//	const auto t2 = dot(p2, plane3Normal);
		//	const auto t3 = dot(p3, plane3Normal);
		//}
		/*
		It might be possible to just calculate the plane the 2 points and the antipodal point are in and then use that for checking which side is it on. The issue would be how to determine the correct orientation. Then instead of sending the 4d plane though 0. It would sent the plane passing though the projected points. One way to calculate this plane might be to fist calculate the original plane (could be precomputed) and then calculate the 3d plane (using a cross product) and the compare the signs of the 2 planes.
		*/

		// Move a vertex of the polygon to some vertex of the sphere, because the deviation from the sphere is the smallest at he vertices and biggest at the centers of the faces.
		//const auto anyVertex = icosahedronVertices[0];
		const auto anyVertex = renderer.sphereLodCenter;
		const auto rotation = unitSphereRotateAToB(
			(sp0 - sphere.center).normalized(),
			anyVertex.normalized());

		const auto transform =
			Mat4::translation(sphere.center) *
			Mat4(rotation.toMatrix()) *
			Mat4(Mat3::scale(sphere.radius));
		renderer.renderSphericalPolygon(sphere.radius, transform, n0, n1, n2, planeNormal);
	};

	auto renderSphericalTriangle = [&](Vec3 sp0, const Sphere& sphere, Vec4 n0, Vec4 n1, Vec4 n2, Vec4 planeNormal) {
		renderSphericalQuad(sp0, sphere, n0, n1, n2, n2, planeNormal);
	};

	auto renderTriangle = [&](Vec4 p0, Vec4 p1, Vec4 p2, Vec4 planeNormal4) {
		const auto p3 = -p0;
		const auto sp0 = stereographicProjection(p0);
		const auto sp1 = stereographicProjection(p1);
		const auto sp2 = stereographicProjection(p2);
		const auto sp3 = stereographicProjection(p3);

		/*renderer.sphere(sp0, width * 3.0f, Color3::RED);
		renderer.sphere(sp1, width * 3.0f, Color3::RED);
		renderer.sphere(sp2, width * 3.0f, Color3::RED);
		renderer.sphere(sp3, width * 3.0f, Color3::GREEN);*/

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
		if (finitePoints.size() == 4) {
			/*
			Tried computing all the planes by choosing triples of points and then comparing the distance to the 4th point to find the best one and the rendering a plane if the 4th points is close enough. This isn't really a good metric, because the plane can still be far away from the edges of the polygon even if it's close to the vertices.

			Could use an alternative fitting method like least squares, but it also probably doesn't make sense, because the user doesn't see the 4th point with relation to the other points. The 4th point is only a helper point to construct the sphere. It doesn't lie on the polygon.

			So it seems like a good metric might finding the maximum deviation of the edges from a plane. 
			Not sure if this is correct, but is seems to me that the maximum distance would happen at the midpoint of the circle curve. So it would make sense to calculate the max of the distances of these midpoints to the plane.

			It might also be good to scale the importance based on the distance from the camera, because objects further away appear smaller so errors are less noticible.
			*/

			const auto planeThoughPolygonVertices = Plane::fromPoints(sp0, sp1, sp2);
			auto deviationFromPlane = [&planeThoughPolygonVertices](Vec4 e0, Vec4 e1) -> f32 {
				const auto s = StereographicSegment::fromEndpoints(e0, e1);
				switch (s.type) {
					using enum StereographicSegment::Type;
				case LINE:
					return 0.0f;
				case CIRCULAR:
					const auto circularMid = s.circular.sample(s.circular.angle / 2.0f);
					return planeThoughPolygonVertices.distance(circularMid);
				}
			};
			const auto deviation = std::max(
				deviationFromPlane(p0, p1),
				std::max(
					deviationFromPlane(p1, p2), deviationFromPlane(p2, p0)
				)
			);
			static float maxAllowedDeviation = 0.005f;
			ImGui::SliderFloat("max allowed deviation", &maxAllowedDeviation, 0.0, 0.03f);

			// Could check if the deviation is less than the radius of the tubes.
			if (deviation < maxAllowedDeviation) {
				renderPlaneTriangle(planeThoughPolygonVertices, n0, n1, n2, planeNormal4);
			} else {
				const auto sphere = Sphere::thoughPoints(sp0, sp1, sp2, sp3);
				renderSphericalTriangle(sp0, sphere, n0, n1, n2, planeNormal4);
			}
		}
	};

	auto renderQuad = [&](Vec4 p0, Vec4 p1, Vec4 p2, Vec4 p3, Vec4 planeNormal4) {
		const auto p3 = -p0;
		const auto sp0 = stereographicProjection(p0);
		const auto sp1 = stereographicProjection(p1);
		const auto sp2 = stereographicProjection(p2);
		const auto sp3 = stereographicProjection(p3);

		/*renderer.sphere(sp0, width * 3.0f, Color3::RED);
		renderer.sphere(sp1, width * 3.0f, Color3::RED);
		renderer.sphere(sp2, width * 3.0f, Color3::RED);
		renderer.sphere(sp3, width * 3.0f, Color3::GREEN);*/

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
		if (finitePoints.size() == 4) {
			/*
			Tried computing all the planes by choosing triples of points and then comparing the distance to the 4th point to find the best one and the rendering a plane if the 4th points is close enough. This isn't really a good metric, because the plane can still be far away from the edges of the polygon even if it's close to the vertices.

			Could use an alternative fitting method like least squares, but it also probably doesn't make sense, because the user doesn't see the 4th point with relation to the other points. The 4th point is only a helper point to construct the sphere. It doesn't lie on the polygon.

			So it seems like a good metric might finding the maximum deviation of the edges from a plane. 
			Not sure if this is correct, but is seems to me that the maximum distance would happen at the midpoint of the circle curve. So it would make sense to calculate the max of the distances of these midpoints to the plane.

			It might also be good to scale the importance based on the distance from the camera, because objects further away appear smaller so errors are less noticible.
			*/

			const auto planeThoughPolygonVertices = Plane::fromPoints(sp0, sp1, sp2);
			auto deviationFromPlane = [&planeThoughPolygonVertices](Vec4 e0, Vec4 e1) -> f32 {
				const auto s = StereographicSegment::fromEndpoints(e0, e1);
				switch (s.type) {
					using enum StereographicSegment::Type;
				case LINE:
					return 0.0f;
				case CIRCULAR:
					const auto circularMid = s.circular.sample(s.circular.angle / 2.0f);
					return planeThoughPolygonVertices.distance(circularMid);
				}
			};
			const auto deviation = std::max(
				deviationFromPlane(p0, p1),
				std::max(
					deviationFromPlane(p1, p2), deviationFromPlane(p2, p0)
				)
			);
			static float maxAllowedDeviation = 0.005f;
			ImGui::SliderFloat("max allowed deviation", &maxAllowedDeviation, 0.0, 0.03f);

			// Could check if the deviation is less than the radius of the tubes.
			if (deviation < maxAllowedDeviation) {
				renderPlaneQuad(planeThoughPolygonVertices, n0, n1, n2, n3, planeNormal4);
			} else {
				const auto sphere = Sphere::thoughPoints(sp0, sp1, sp2, sp3);
				renderSphericalQuad(sp0, sphere, n0, n1, n2, n3, planeNormal4);
			}
		}
	};

	//static i32 faceI = 0;
	auto renderFace = [&](i32 faceI, const Cell& cell) {
		const auto& face = faces[faceI];
		const auto& p0 = transformedVertices4[face.vertices[0]];
		const auto& p1 = transformedVertices4[face.vertices[1]];
		const auto& p2 = transformedVertices4[face.vertices[2]];
		Vec4 normal(0.0f);
		for (i32 i = 0; i < cell.faces.size(); i++) {
			if (cell.faces[i] == faceI) {
				normal = cell.faceNormals[i];
			}
		}
		renderTriangle(p0, p1, p2, normal);
		
		const auto faceCenter = ((p0 + p1 + p2) / 3.0f).normalized();
		const auto transformedFaceCenter = stereographicProjection(faceCenter);
		const auto transformedNormal = stereographicProjectionJacobian(faceCenter, view4 * normal);
		renderer.sphere(transformedFaceCenter, 0.03f, Color3::RED);
		renderer.line(transformedFaceCenter, transformedFaceCenter + transformedNormal.normalized(), 0.02f, Color3::MAGENTA);
	};

	//auto& cell = cells[0];
	//for (const auto& face : cell.faces) {
	//	renderFace(face, cell);
	//	//renderFace(1);
	//}

	renderer.coloredShadingTrianglesAddMesh(lineGenerator, Color3::WHITE);
	lineGenerator.reset();
	renderer.renderInfinitePlanes();
	renderer.renderSphericalPolygons();

	renderer.renderColoredShadingTriangles(ColoredShadingInstance{
		.model = Mat4::identity
	});


	renderer.renderCyllinders();
	renderer.renderHemispheres();
}

void Visualization2::lodLevelsSettingsGui() {
	for (auto& level : lodLevelsSettings) {
		ImGui::PushID(&level);
		ImGui::SliderFloat("min radius", &level.minRadius, 0.0f, 40.0f);
		ImGui::InputInt("division count", &level.divisionCount);
		ImGui::PopID();
		ImGui::Separator();
	}
	if (ImGui::Button("reload")) {
		renderer.generateSphereLods(lodLevelsSettings);
	}
}
