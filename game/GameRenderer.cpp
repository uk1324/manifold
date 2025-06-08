#include "GameRenderer.hpp"
#include <gfx/ShaderManager.hpp>
#include <engine/Window.hpp>
#include <engine/Math/Color.hpp>
#include <game/Math.hpp>
#include <imgui/imgui.h>
#include <engine/Math/Interpolation.hpp>
#include <game/Polyhedra.hpp>
#include <StructUtils.hpp>
#include <engine/Math/Constants.hpp>
#include <game/Shaders/transparencyCompositingData.hpp>
#include <game/Shaders/texturedFullscreenQuadData.hpp>

template<typename Vertex>
void renderTriangles(ShaderProgram& shader, TriangleRenderer<Vertex>& r) {
	if (r.vertices.size() == 0 || r.indices.size() == 0) {
		return;
	}
	r.vbo.allocateData(r.vertices.data(), r.vertices.size() * sizeof(Vertex));
	r.ibo.allocateData(r.indices.data(), r.indices.size() * sizeof(u32));

	shader.use();
	r.vao.bind();
	glDrawElements(GL_TRIANGLES, i32(r.indices.size()), GL_UNSIGNED_INT, nullptr);

	r.vertices.clear();
	r.indices.clear();
}

template<typename Vertex, typename Instance>
void renderTriangles(ShaderProgram& shader, TriangleRenderer<Vertex>& r, Vbo& instancesVbo, const Instance& instance) {
	if (r.vertices.size() == 0 || r.indices.size() == 0) {
		return;
	}
	r.vbo.allocateData(r.vertices.data(), r.vertices.size() * sizeof(Vertex));
	r.ibo.allocateData(r.indices.data(), r.indices.size() * sizeof(u32));

	shader.use();
	drawInstances(
		r.vao,
		instancesVbo,
		constView(instance),
		[&](usize count) {
			glDrawElements(GL_TRIANGLES, i32(r.indices.size()), GL_UNSIGNED_INT, nullptr);
		}
	);

	r.vertices.clear();
	r.indices.clear();
}


template<typename Shader, typename Vertex>
Mesh makeMesh(View<const Vertex> vertices, View<const i32> indices, Vbo& instancesVbo) {
	auto vbo = Vbo(vertices.data(), vertices.size() * sizeof(Vertex));
	auto ibo = Ibo(indices.data(), indices.size() * sizeof(i32));
	auto vao = createInstancingVao<Shader>(vbo, ibo, instancesVbo);
	return Mesh{
		MOVE(vbo),
		MOVE(ibo),
		MOVE(vao),
		.indexCount = i32(indices.size())
	};
}

template<typename Instance>
void drawMeshInstances(Mesh& mesh, View<const Instance> instances, Vbo& instancesVbo) {
	drawInstances(mesh.vao, instancesVbo, instances, [&](GLsizei count) {
		glDrawElementsInstanced(GL_TRIANGLES, GLsizei(mesh.indexCount), GL_UNSIGNED_INT, nullptr, count);
	});
}

#include <game/DoublyConnectedEdgeList.hpp>
#include <iostream>

// Uses homogeneous coordinates
// The only difference between trasforming a normal point and a point at infinity is that points at infinity don't get translated, because the w basis in the matrix gets multiplied by zero.
// When using a perspective matrix the w coorindate of the output vector is indepentent of the w coorindate of the input vector. The projection matrix just copies the the z value. So setting w = 0 doesn't cause division by zero.
HomogenousVertex infinitePlaneVertices[]{
	// I don't think it's possible to draw an infinite plane using only 2 triangles. I think that might be because the vertices don't get translated so the same thing is rendered no matter the camera position. If you actually try it you will see something like a flickering 1D checkboard. This might be because the view direction is always parallel with the plane.
	{ Vec4(0, 0, 0, 1) },
	{ Vec4(1, 0, 0, 0) },
	{ Vec4(0, 0, 1, 0) },
	{ Vec4(-1, 0, 0, 0) },
	{ Vec4(0, 0, -1, 0) },
};

i32 infinitePlaneIndices[]{
	0, 1, 2,
	0, 2, 3,
	0, 3, 4,
	0, 4, 1
};

#define FONT_FOLDER "engine/assets/fonts"

GameRenderer GameRenderer::make() {
	auto instancesVbo = Vbo(1024ull * 20);

	auto makeColoredShadedMesh = [&instancesVbo](const std::vector<Vertex3Pn>& vertices, const std::vector<i32>& indices) {
		return makeMesh<ColoredShader>(constView(vertices), constView(indices), instancesVbo);
	};
	std::vector<Vertex3Pn> vertices;
	std::vector<i32> indices;
	auto coloredShaderMesh = [&vertices, &indices, &makeColoredShadedMesh]() -> Mesh {
		return makeColoredShadedMesh(vertices, indices);
	};

	auto meshClear = [&]() {
		vertices.clear();
		indices.clear();
	};

	const i32 circleVertexCount = 50;

	{
		const i32 hemisphereVertexCount = 20;
		meshClear();
		for (i32 ui = 0; ui < hemisphereVertexCount; ui++) {
			for (i32 vi = 0; vi < hemisphereVertexCount; vi++) {
				const auto u = f32(ui) / f32(hemisphereVertexCount) * TAU<f32>;
				const auto v = f32(vi) / f32(hemisphereVertexCount - 1) * (PI<f32> / 2.0f);
				const auto pos = Vec3(cos(u) * cos(v), sin(u) * cos(v), sin(v)).normalized();
				const auto& normal = pos;
				vertices.push_back(Vertex3Pn{ pos, normal });
			}
		}

		auto toIndex = [](i32 ui, i32 vi) {
			return ui * hemisphereVertexCount + vi;
		};
		i32 previousUi = hemisphereVertexCount - 1;
		for (i32 ui = 0; ui < hemisphereVertexCount; ui++) {
			for (i32 vi = 0; vi < hemisphereVertexCount - 1; vi++) {
				indicesAddQuad(
					indices,
					toIndex(previousUi, vi),
					toIndex(previousUi, vi + 1),
					toIndex(ui, vi + 1),
					toIndex(ui, vi)
				);
			}
			previousUi = ui;
		}
	}
	auto hemisphere = coloredShaderMesh();

	{
		meshClear();
		for (i32 i = 0; i < circleVertexCount; i++) {
			const auto a = f32(i) / f32(circleVertexCount) * TAU<f32>;
			const auto position = Vec3(cos(a), sin(a), 0.0f);
			const auto normal = Vec3(position.x, position.y, 1.0f) / sqrt(2.0f);
			vertices.push_back(Vertex3Pn{
				.position = position,
				.normal = normal
			});
		}
		const auto topVerticiesOffset = i32(vertices.size());
		const auto centerAngleOffset = TAU<f32> / f32(circleVertexCount) / 2.0f;
		for (i32 i = 0; i < circleVertexCount; i++) {
			const auto a = f32(i) / f32(circleVertexCount) * TAU<f32> +centerAngleOffset;
			const auto normal = Vec3(cos(a), sin(a), 1.0f) / sqrt(2.0f);
			vertices.push_back(Vertex3Pn{
				.position = Vec3(0.0f, 0.0f, 1.0f),
				.normal = normal
			});
		}

		i32 previous = circleVertexCount - 1;
		for (i32 i = 0; i < circleVertexCount; i++) {
			indicesAddTri(indices, previous, i, topVerticiesOffset + i);
			previous = i;
		}
	}
	auto coneMesh = coloredShaderMesh();

	{
		meshClear();
		Vec3 normal(0.0f, 0.0f, 1.0f);
		for (i32 i = 0; i < circleVertexCount; i++) {
			const auto a = f32(i) / f32(circleVertexCount) * TAU<f32>;
			const auto position = Vec3(cos(a), sin(a), 0.0f);
			vertices.push_back(Vertex3Pn{
				.position = position,
				.normal = normal
			});
		}
		const auto centerIndex = i32(vertices.size());
		vertices.push_back(Vertex3Pn{
			.position = Vec3(0.0f),
			.normal = normal
		});

		i32 previous = circleVertexCount;
		for (i32 i = 0; i < circleVertexCount; i++) {
			indicesAddTri(indices, previous, i, centerIndex);
			previous = i;
		}
	}
	auto circleMesh = coloredShaderMesh();

	{
		meshClear();
		for (i32 i = 0; i < circleVertexCount; i++) {
			const auto t = f32(i) / f32(circleVertexCount);
			const auto a = lerp(0.0f, TAU<f32>, t);
			const auto bottomPos = Vec3(cos(a), sin(a), 0.0f);
			const auto& normal = bottomPos;
			const auto topPos = Vec3(bottomPos.x, bottomPos.y, 1.0f);
			vertices.push_back(Vertex3Pn{ .position = bottomPos, .normal = normal });
			vertices.push_back(Vertex3Pn{ .position = topPos, .normal = normal });
		}
		i32 previous = circleVertexCount - 1;
		for (i32 i = 0; i < circleVertexCount; i++) {
			const auto vBottom0 = previous * 2;
			const auto vTop0 = previous * 2 + 1;
			const auto vBottom1 = i * 2;
			const auto vTop1 = i * 2 + 1;
			indicesAddQuad(indices, vBottom0, vBottom1, vTop1, vTop0);
			previous = i;
		}
	}
	auto cyllinderMesh = coloredShaderMesh();

	{
		meshClear();
		const auto data = flatShadeRegularPolyhedron(constView(cubeVertices), constView(cubeFaces), cubeVerticesPerFace);
		for (i32 i = 0; i < data.positions.size(); i++) {
			vertices.push_back(Vertex3Pn{ data.positions[i], data.normals[i] });
		}
		for (i32 i = 0; i < data.indices.size(); i++) {
			indices.push_back(data.indices[i]);
		}
	}
	auto cubeMesh = coloredShaderMesh();


	std::vector<SphereImpostorVertex> sphereImpostorMeshVertices;
	std::vector<i32> sphereImpostorMeshIndices;
	{
		// Could do without flat shading, just pass the positions
		const auto data = flatShadeRegularPolyhedron(constView(cubeVertices), constView(cubeFaces), cubeVerticesPerFace);
		for (i32 i = 0; i < data.positions.size(); i++) {
			sphereImpostorMeshVertices.push_back(SphereImpostorVertex{ data.positions[i] });
		}
		for (i32 i = 0; i < data.indices.size(); i++) {
			sphereImpostorMeshIndices.push_back(data.indices[i]);
		}
	}
	auto sphereImpostorMesh = makeMesh<SphereImpostor2Shader>(constView(sphereImpostorMeshVertices), constView(sphereImpostorMeshIndices), instancesVbo);

	{
		sphereImpostorMeshVertices.clear();
		sphereImpostorMeshIndices.clear();
		sphereImpostorMeshVertices.push_back(SphereImpostorVertex{ Vec3(0.0f) });
		sphereImpostorMeshVertices.push_back(SphereImpostorVertex{ Vec3(1.0f, 0.0f, 0.0f) });
		sphereImpostorMeshVertices.push_back(SphereImpostorVertex{ Vec3(0.0f, 1.0f, 0.0f) });
		sphereImpostorMeshIndices.push_back(0);
		sphereImpostorMeshIndices.push_back(1);
		sphereImpostorMeshIndices.push_back(2);
	}
	auto sphereImpostorMeshTri = makeMesh<SphereImpostorShader>(constView(sphereImpostorMeshVertices), constView(sphereImpostorMeshIndices), instancesVbo);

	auto text3QuadMesh = [&] {
		Vertex3P quad3Vertices[]{
			Vertex3P{ Vec3(-1.0f, 1.0f, 0.0f) },
			Vertex3P{ Vec3(1.0f, 1.0f, 0.0f) },
			Vertex3P{ Vec3(-1.0f, -1.0f, 0.0f) },
			Vertex3P{ Vec3(1.0f, -1.0f, 0.0f) }
		};
		i32 quad3Indices[]{ 0, 1, 2, 2, 1, 3 };
		return makeMesh<Text3Shader>(constView(quad3Vertices), constView(quad3Indices), instancesVbo);
	}();


	auto gfx2d = Gfx2d::make();

	auto quadPtVao = createInstancingVao<TexturedFullscreenQuadShader>(gfx2d.quad2dPtVbo, gfx2d.quad2dPtIbo, instancesVbo); 

	GameRenderer renderer{
		.transform = Mat4::identity,
		.view = Mat4::identity,
		.projection = Mat4::identity,
		.coloredShader = MAKE_GENERATED_SHADER(COLORED),
		MOVE(hemisphere),
		MOVE(coneMesh),
		MOVE(circleMesh),
		MOVE(cyllinderMesh),
		MOVE(cubeMesh),
		.coloredShadingTriangles = TriangleRenderer<Vertex3Pnc>::make<ColoredShadingShader>(instancesVbo),
		.coloredShadingShader = MAKE_GENERATED_SHADER(COLORED_SHADING),
		.homogenousShader = MAKE_GENERATED_SHADER(HOMOGENOUS),
		.infinitePlaneMesh = makeMesh<HomogenousShader>(constView(infinitePlaneVertices), constView(infinitePlaneIndices), instancesVbo),
		.sphereImpostorsShader = MAKE_GENERATED_SHADER(SPHERE_IMPOSTOR),
		.sphereImpostors2Shader = MAKE_GENERATED_SHADER(SPHERE_IMPOSTOR_2),
		MOVE(sphereImpostorMesh),
		MOVE(sphereImpostorMeshTri),
		MOVE(text3QuadMesh),
		.text3Shader = MAKE_GENERATED_SHADER(TEXT_3),
		.font = Font::loadSdfWithCachingAtDefaultPath(FONT_FOLDER, "RobotoMono-Regular"),
		MOVE(gfx2d),
		MOVE(instancesVbo),
	};

	return renderer;
}

void GameRenderer::frameUpdate(Mat4 view, Vec3 cameraPosition, const StereographicCamera& stereographicCamera) {
	this->viewInverse4 = stereographicCamera.view4Inversed();
	this->cameraPos4 = stereographicCamera.pos4();
	const auto cameraForward = (Vec4(Vec3::FORWARD, 0.0f) * view.inversed()).xyz().normalized();
	const auto aspectRatio = Window::aspectRatio();
	//const auto projection = Mat4::perspective(PI<f32> / 2.0f, aspectRatio, 0.1f, 1000.0f);
	const auto projection = Mat4::perspective(PI<f32> / 2.0f, aspectRatio, 0.01f, 200.0f);
	this->transform = projection * view;
	this->view = view;
	this->projection = projection;
	this->cameraForward = cameraForward;
	this->cameraPosition = cameraPosition;

	glViewport(0, 0, i32(Window::size().x), i32(Window::size().y));

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_BLEND);
}


void GameRenderer::initColoredShader() {
	coloredShader.use();
	shaderSetUniforms(coloredShader, ColoredVertUniforms{
		.transform = transform,
		.view = view
	});
}

void GameRenderer::sphere(Vec3 center, f32 radius, Vec3 color) {
	const auto translateScale = Mat4::translation(center) * Mat4(Mat3::scale(radius, radius, -radius));
	hemispheres.push_back(ColoredInstance{
		.color = color,
		.model = translateScale
	});
	hemispheres.push_back(ColoredInstance{
		.color = color,
		.model = translateScale * Mat4(Mat3::scale(1.0f, 1.0f, -1.0f))
	});
}

void GameRenderer::renderHemispheres() {
	initColoredShader();
	drawMeshInstances(hemisphere, constView(hemispheres), instancesVbo);
	hemispheres.clear();
}

// Transforms a radially symmetric mesh such that (0, 0, 0) is mapped to a and (0, 0, 1) is mapped to (b - a).normalized().
Mat4 transformMesh(Vec3 a, Vec3 b) {
	auto v0 = b - a;
	const auto length = v0.length();
	v0 /= length;
	const auto v1 = anyPerpendicularVector(v0);
	const auto v2 = cross(v0, v1).normalized();
	const auto rotateTranslate = Mat4::translation(a) * Mat4(Mat3(v1, v2, v0));
	return rotateTranslate;
}

void GameRenderer::cone(Vec3 bottom, Vec3 top, f32 radius, Vec3 color) {
	const auto rotateTranslate = transformMesh(bottom, top);
	cones.push_back(ColoredInstance{
		.color = color,
		.model = rotateTranslate * Mat4(Mat3::scale(Vec3(1.0f, 1.0f, (bottom - top).length())))
	});
}

void GameRenderer::renderCones() {
	initColoredShader();
	drawMeshInstances(coneMesh, constView(cones), instancesVbo);
	cones.clear();
}

void GameRenderer::renderCircles() {
	initColoredShader();
	drawMeshInstances(circleMesh, constView(circles), instancesVbo);
	circles.clear();
}

void GameRenderer::renderCyllinders() {
	initColoredShader();
	drawMeshInstances(cyllinderMesh, constView(cyllinders), instancesVbo);
	cyllinders.clear();
}

void GameRenderer::cube(Vec3 color) {
	cubes.push_back(ColoredInstance{
		.color = color,
		.model = Mat4::identity
	});
}

void GameRenderer::renderCubes() {
	initColoredShader();
	drawMeshInstances(cubeMesh, constView(cubes), instancesVbo);
	cubes.clear();
}

void GameRenderer::line(Vec3 a, Vec3 b, f32 radius, Vec3 color, bool caps) {
	const auto length = (b - a).length();
	const auto rotateTranslate = transformMesh(a, b);
	const auto model = rotateTranslate * Mat4(Mat3::scale(Vec3(radius, radius, length)));
	cyllinders.push_back(ColoredInstance{
		.color = color,
		.model = model
	});
	if (caps) {
		const auto hemisphereScale = Mat4(Mat3::scale(Vec3(radius)));
		hemispheres.push_back(ColoredInstance{
			.color = color,
			.model = rotateTranslate * Mat4::translation(Vec3(0.0f, 0.0f, length)) * Mat4(Mat3::scale(radius))
		});
		hemispheres.push_back(ColoredInstance{
			.color = color,
			.model = rotateTranslate * Mat4(Mat3::scale(Vec3(radius, radius, -radius)))
		});
	}
}

void GameRenderer::renderColoredShadingTriangles(const ColoredShadingInstance& instance) {
	shaderSetUniforms(coloredShadingShader, ColoredShadingVertUniforms{
		.transform = transform,
		.view = view,
	});
	//.model = coloredShadingModel,
	renderTriangles(coloredShadingShader, coloredShadingTriangles, instancesVbo, instance);
}

void GameRenderer::coloredShadingTrianglesAddMesh(const std::vector<Vec3>& positions, const std::vector<Vec3>& normals, const std::vector<i32>& indices, Vec3 color) {
	const auto offset = i32(coloredShadingTriangles.vertices.size());
	for (i32 i = 0; i < positions.size(); i++) {
		coloredShadingTriangles.addVertex(Vertex3Pnc{
			.position = positions[i],
			.normal = normals[i],
			.color = Vec4(color, 1.0f),
		});
	}
	for (const auto& index : indices) {
		coloredShadingTriangles.indices.push_back(offset + index);
	}
}

void GameRenderer::coloredShadingTrianglesAddMesh(const LineGenerator& lineGenerator, Vec3 color) {
	coloredShadingTrianglesAddMesh(lineGenerator.positions, lineGenerator.normals, lineGenerator.indices, color);
}

void GameRenderer::renderInfinitePlanes() {
	homogenousShader.use();
	shaderSetUniforms(
		homogenousShader,
		HomogenousVertUniforms{
			.transform = transform
		}
	);

	shaderSetUniforms(
		homogenousShader,
		HomogenousFragUniforms{
			.screenSize = Window::size(),
			.inverseTransform = transform.inversed(),
			.cameraPos = cameraPos4,
			.viewInverse4 = viewInverse4
		}
	);
	drawMeshInstances(infinitePlaneMesh, constView(infinitePlanes), instancesVbo);
	infinitePlanes.clear();
}


void GameRenderer::sphereImpostor(Mat4 transform, Vec3 position, f32 radius, Vec4 n0, Vec4 n1, Vec4 n2, Vec4 n3, Vec4 n4, Vec4 planeNormal) {
	sphereImpostors.push_back(SphereImpostorInstance{
		.transform = transform,
		.sphereCenter = position,
		.sphereRadius = radius,
		.n0 = n0,
		.n1 = n1,
		.n2 = n2,
		.n3 = n3,
		.n4 = n4,
		.planeNormal = planeNormal
	});
}

void GameRenderer::sphereImpostorCube(Vec4 center4, Mat4 transform, Vec3 position, f32 radius, Vec4 n0, Vec4 n1, Vec4 n2, Vec4 n3, Vec4 planeNormal) {

	sphereImpostorsCubes.push_back(SphereImpostor2Instance{
		.transform = transform,
		.sphereCenter = position,
		.sphereCenter4 = center4,
		.sphereRadius = radius,
		.n0 = n0,
		.n1 = n1,
		.n2 = n2,
		.n3 = n3,
		.planeNormal = planeNormal,
	});
}

void GameRenderer::renderSphereImpostors() {
	shaderSetUniforms(
		sphereImpostorsShader,
		SphereImpostorVertUniforms{
			.transform = transform,
		}
	);
	shaderSetUniforms(
		sphereImpostorsShader,
		SphereImpostorFragUniforms{
			.cameraPos = cameraPosition,
			.cameraPos4 = cameraPos4,
			.viewInverse4 = viewInverse4,
		}
	);
	sphereImpostorsShader.use();
	if (useImpostorsTriangles) {
		drawMeshInstances(sphereImpostorMeshTri, constView(sphereImpostors), instancesVbo);
	} else {
		drawMeshInstances(sphereImpostorMesh, constView(sphereImpostors), instancesVbo);
	}
	sphereImpostors.clear();

	//drawMeshInstances(sphereImpostorMesh, constView(sphereImpostorsCubes), instancesVbo);
	//sphereImpostorsCubes.clear();

	{
		shaderSetUniforms(
			sphereImpostors2Shader,
			SphereImpostor2VertUniforms{
				.transform = transform,
			}
		);
		shaderSetUniforms(
			sphereImpostors2Shader,
			SphereImpostor2FragUniforms{
				.cameraPos = cameraPosition,
				.cameraPos4 = cameraPos4,
				.viewInverse4 = viewInverse4,
			}
		);
		sphereImpostors2Shader.use();
		drawMeshInstances(sphereImpostorMesh, constView(sphereImpostorsCubes), instancesVbo);
		sphereImpostorsCubes.clear();
	}
}

void GameRenderer::stereographicLineSegment(Vec4 e0, Vec4 e1, f32 width, bool scaleWidth) {
	const auto segment = StereographicSegment::fromEndpoints(e0, e1);
	stereographicLineSegment(segment, width, scaleWidth);
}

void GameRenderer::stereographicLineSegment(const StereographicSegment& segment, f32 width, bool scaleWidth) {
	switch (segment.type) {
		using enum StereographicSegment::Type;

	case LINE: {
		//const auto& p0 = segment.line.e[0];
		//const auto& p1 = segment.line.e[1];
		//if (isPointAtInfinity(p0) && isPointAtInfinity(p1)) {
		//	CHECK_NOT_REACHED();
		//	return;
		//}
		//if (isPointAtInfinity(p0) || isPointAtInfinity(p1)) {
		//	auto atInfinity = p0;
		//	auto finite = p1;
		//	if (isPointAtInfinity(finite)) {
		//		std::swap(atInfinity, finite);
		//	}
		//	const auto direction = finite.normalized();
		//	line(finite, finite + direction * 1000.0f, width, Color3::WHITE);
		//}
		break;
	}

	case CIRCULAR: {
		auto& s = segment.circular;
		if (scaleWidth) {
			lineGenerator.addStereographicArc(segment, width);
		} else {
			const auto e0 = s.center + s.start;
			const auto e1 = s.sample(s.angle);
			//line(e0, e1, width, Color3::WHITE);
			if (s.initialVelocity.length() > 200.0f) {
				line(e0, e1, width, Color3::WHITE);
			} else {
				//lineGenerator.addCircularArc(s.start, s.initialVelocity, s.center, s.angle, width);
			}
		}
		break;
	}

	}
}

void GameRenderer::planeTriangle(const Plane& plane, Vec4 edgeNormal0, Vec4 edgeNormal1, Vec4 edgeNormal2, Vec4 edgeNormal3, Vec4 edgeNormal4, Vec4 planeNormal) {

	Vec3 untransformedPlaneMeshNormal = Vec3(0.0f, 1.0f, 0.0f);
	const auto rotation = unitSphereRotateAToB(untransformedPlaneMeshNormal, plane.n);

	infinitePlanes.push_back(HomogenousInstance{
		.transform =
			Mat4::translation(plane.d * plane.n) *
			Mat4(rotation.inverseIfNormalized().toMatrix()),
		.n0 = edgeNormal0,
		.n1 = edgeNormal1,
		.n2 = edgeNormal2,
		.n3 = edgeNormal3,
		.n4 = edgeNormal4,
		.planeNormal = planeNormal,
	});
}

void GameRenderer::sphericalTriangle(Vec3 sp0, Vec3 sp1, Vec3 sp2, const Sphere& sphere, Vec4 n0, Vec4 n1, Vec4 n2, Vec4 n3, Vec4 n4, Vec4 planeNormal) {
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
	//const auto anyVertex = renderer.sphereLodCenter;
	//const auto rotation = unitSphereRotateAToB(
	//	(sp0 - sphere.center).normalized(),
	//	anyVertex.normalized());

	//const auto transform =
	//	Mat4::translation(sphere.center) *
	//	Mat4(rotation.toMatrix()) *
	//	Mat4(Mat3::scale(sphere.radius));
	//renderer.renderSphericalPolygon(sphere.radius, sphere.center, transform, n0, n1, n2, n3, planeNormal);

	//ImGui::Checkbox("use triangle impostors", &useImpostorsTriangles);
	static f32 impostorsTriangleScale = 1.2f;
	if (useImpostorsTriangles) {
		//ImGui::SliderFloat("impostorsTriangleScale", &impostorsTriangleScale, 1.0f, 2.0f);
	}

	if (useImpostorsTriangles) {
		// map (0, 0, 0) -> v0, (1, 0, 0) -> v1, (0, 1, 0) -> v2
		auto transformTriangle = [](Vec3 v0, Vec3 v1, Vec3 v2) -> Mat4 {
			const auto center = (v0 + v1 + v2) / 3.0f;
			auto t = [&](Vec3& v) {
				v -= center;
				//v *= 2.0f;
				v *= impostorsTriangleScale;
				v += center;
				};
			t(v0);
			t(v1);
			t(v2);
			// The 4th coordinate of all these points is 1.
			return Mat4(
				Vec4(v0 - v2, 0.0f),
				Vec4(v1 - v2, 0.0f),
				//Vec4(0.0f),
				Vec4(0.0f, 0.0f, 0.0f, 0.0f),
				Vec4(v2, 1.0f)
			);
			};
		sphereImpostor(transformTriangle(sp0, sp1, sp2), sphere.center, sphere.radius, n0, n1, n2, n3, n4, planeNormal);
		/*renderer.sphereImpostor(transformTriangle(sp0, sp2, sp3), sphere.center, sphere.radius, n0, n1, n2, n3, planeNormal);*/
	} else {
		const auto transform =
			Mat4::translation(sphere.center) *
			Mat4(Mat3::scale(sphere.radius));
		sphereImpostor(transform, sphere.center, sphere.radius, n0, n1, n2, n3, n4, planeNormal);
	}
}

void GameRenderer::stereographicTriangle(Vec4 p0, Vec4 p1, Vec4 p2, Vec4 planeNormal4, Vec4 edgeNormal0, Vec4 edgeNormal1, Vec4 edgeNormal2, Vec4 edgeNormal3, Vec4 edgeNormal4) {
	const auto p4 = -p0;
	const auto sp0 = stereographicProjection(p0);
	const auto sp1 = stereographicProjection(p1);
	const auto sp2 = stereographicProjection(p2);
	const auto sp3 = stereographicProjection(p4);

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

	/*auto& face = faces[faceI];
	const auto n0 = view4 * face.edgeNormals[0];
	const auto n1 = view4 * face.edgeNormals[1];
	const auto n2 = view4 * face.edgeNormals[2];*/
	//const auto n3 = view4 * face.edgeNormals[3];

	if (finitePoints.size() == 4) {
		/*
		Tried computing all the planes by choosing triples of points and then comparing the distance to the 4th point to find the best one and the rendering a plane if the 4th points is close enough. This isn't really a good metric, because the plane can still be far away from the edges of the polygon even if it's close to the vertices.

		Could use an alternative fitting method like least squares, but it also probably doesn't make sense, because the user doesn't see the 4th point with relation to the other points. The 4th point is only a helper point to construct the sphere. It doesn't lie on the polygon.

		So it seems like a good metric might finding the maximum deviation of the edges from a plane.
		Not sure if this is correct, but is seems to me that the maximum distance would happen at the midpoint of the circle curve. So it would make sense to calculate the max of the distances of these midpoints to the plane.

		It might also be good to scale the importance based on the distance from the camera, because objects further away appear smaller so errors are less noticible.
		*/

		const auto sphere = Sphere::thoughPoints(sp0, sp1, sp2, sp3);

		auto isInf = [](f32 v) {
			return isinf(v) || isnan(v);
			};

		if (isInf(sphere.radius) || isInf(sphere.center.x) || isInf(sphere.center.y) || isInf(sphere.center.z)) {
			const auto planeThoughPolygonVertices = Plane::fromPoints(sp0, sp1, sp2);
			planeTriangle(planeThoughPolygonVertices, edgeNormal0, edgeNormal1, edgeNormal2, edgeNormal3, edgeNormal4, planeNormal4);
		} else {
			sphericalTriangle(sp0, sp1, sp2, sphere, edgeNormal0, edgeNormal1, edgeNormal2, edgeNormal3, edgeNormal4, planeNormal4);
		}
	}
}

void GameRenderer::stereographicSphere(Vec4 pos, f32 radius) {
	const auto s = ::stereographicSphere(pos, radius);
	const auto transform =
		Mat4::translation(s.center) *
		Mat4(Mat3::scale(s.radius));
	sphereImpostorCube(pos, transform, s.center, s.radius, Vec4(0.0f), Vec4(0.0f), Vec4(0.0f), Vec4(0.0f), Vec4(0.0f));
}



Vec2 textCenteredPosition(const Font& font, Vec2 center, f32 maxHeight, std::string_view text) {
	/*const auto info = font.textInfo(maxHeight, text, Constants::additionalTextSpacing);*/
	const auto info = font.textInfo(maxHeight, text);
	Vec2 position = center;
	position.y -= info.bottomY;
	position -= info.size / 2.0f;
	return position;
}

void GameRenderer::centertedText(Vec3 center, f32 size, std::string_view text, Vec3 color) {
	//const auto toUiSpace = Mat3x2::scale(Vec2(2.0f)) * gfx.camera.worldToCameraToNdc();
	Vec2 center2(0.0f);
	Vec2 bottomLeftPosition = textCenteredPosition(font, center2, size, text);

	const auto right = view[0].xyz();
	const auto up = view[2].xyz();
	const auto forward = view[3].xyz();
	TextRenderInfoIterator iterator(font, bottomLeftPosition, Mat3x2::identity, size, text);
	for (auto info = iterator.next(); info.has_value(); info = iterator.next()) {
		const auto& t = info->transform;
		const auto t3 = Mat4(
			Vec4(t[0][0], t[0][1], 0.0f, 0.0f),
			Vec4(t[1][0], t[1][1], 0.0f, 0.0f),
			Vec4(0.0f, 0.0f, 1.0f, 0.0f),
			Vec4(t[2][0], t[2][1], 0.0f, 1.0f)
		);
		/*const auto transform = this->transform * t3 * view.inversed().removedTranslation();*/
		//const auto transform = this->transform * Mat4::translation(Vec3(0.0f, 0.0f, 1.0f)) * Mat4(Mat3::scale(0.1f));
		//const auto transform = this->transform * Mat4::translation(center) * Mat4(Mat3::scale(0.1f));
		const auto transform = this->transform * Mat4::translation(center) * view.inversed().removedTranslation() * t3;
		//const auto transform = projection * view;
		//const auto transform = projection * view * t3 * view.inversed().removedTranslation();
		/*Vec2 pos2 = Vec2(0.0f) * info->transform;
		Vec3 pos3 = center + pos2.x * right;*/


		text3Instances.push_back(Text3Instance{
			.transform = transform,
			//.transform = info->transform,
			.offsetInAtlas = info->offsetInAtlas,
			.sizeInAtlas = info->sizeInAtlas,
			.color = color,
		});
	}

	//gameText(textCenteredPosition(font, position, maxHeight, text), maxHeight, text, hoverT, color);
	/*flowParticles.push_back(FlowParticleInstance{
		.positionScale = Vec4(position.x, position.y, position.z, size),
		.color = color,
	});*/
}

void GameRenderer::renderText() {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

	text3Shader.use();
	text3Shader.setTexture("fontAtlas", 0, font.fontAtlas);
	//shaderSetUniforms(text3Shader, FlowParticleVertUniforms{
	//	.transform = transform,
	//	.rotate = rotateMatrix
	//	});

	//drawInstances(text3QuadMesh.vao, instancesVbo, constView(text3Instances), drawMeshInstances);
	drawMeshInstances(text3QuadMesh, constView(text3Instances), instancesVbo);
	text3Instances.clear();
	glDisable(GL_BLEND);
}
