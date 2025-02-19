#include "Renderer.hpp"
#include <StructUtils.hpp>
#include <engine/Math/Constants.hpp>
#include <engine/Math/Interpolation.hpp>
#include <gfx/ShaderManager.hpp>
#include <gfx/Instancing.hpp>
#include <engine/Window.hpp>

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

Vec3 anyPerpendicularVector(Vec3 v) {
	v = v.normalized();
	const auto attempt = cross(v, Vec3(1.0f, 0.0f, 0.0f));
	if (attempt.lengthSquared() == 0.0f) {
		return cross(v, Vec3(0.0f, 1.0f, 0.0f)).normalized();
	}
	return attempt.normalized();
}

void indicesAddTri(std::vector<i32>& indicies, i32 i0, i32 i1, i32 i2) {
	indicies.push_back(i0);
	indicies.push_back(i1);
	indicies.push_back(i2);
}

void indicesAddQuad(std::vector<i32>& indicies, i32 i00, i32 i01, i32 i11, i32 i10) {
	/*
	i01-i11
	|  /  |
	i00-i10
	*/
	indicies.push_back(i00);
	indicies.push_back(i10);
	indicies.push_back(i11);

	indicies.push_back(i00);
	indicies.push_back(i11);
	indicies.push_back(i01);
}

template<typename Shader, typename Vertex>
Renderer::Mesh makeMesh(View<const Vertex> vertices, View<const i32> indices, Vbo& instancesVbo) {
	auto vbo = Vbo(vertices.data(), vertices.size() * sizeof(Vertex));
	auto ibo = Ibo(indices.data(), indices.size() * sizeof(i32));
	auto vao = createInstancingVao<Shader>(vbo, ibo, instancesVbo);
	return Renderer::Mesh{
		MOVE(vbo),
		MOVE(ibo),
		MOVE(vao),
		.indexCount = i32(indices.size())
	};
}

template<typename Shader, typename Vertex>
Renderer::Mesh makeMesh(const std::vector<Vertex>& vertices, const std::vector<i32> indices, Vbo& instancesVbo) {
	return makeMesh<Shader>(constView(vertices), constView(indices), instancesVbo);
}

template<typename Instance>
void drawMeshInstances(Renderer::Mesh& mesh, View<const Instance> instances, Vbo& instancesVbo) {
	drawInstances(mesh.vao, instancesVbo, instances, [&](GLsizei count) {
		glDrawElementsInstanced(GL_TRIANGLES, GLsizei(mesh.indexCount), GL_UNSIGNED_INT, nullptr, count);
	});
}

Renderer Renderer::make() {
	auto instancesVbo = Vbo(1024ull * 10);

	auto makeRectVertex = [&](f32 x, f32 y) {
		return Vertex3Pnt{ Vec3(x, y, 0.0f), Vec3(0.0f, 0.0f, 1.0f), Vec2(x, y) };
	};
	Vertex3Pnt rectVertices[]{
		makeRectVertex(0.0f, 0.0f),
		makeRectVertex(1.0f, 0.0f),
		makeRectVertex(1.0f, 1.0f),
		makeRectVertex(0.0f, 1.0f)
	};
	i32 rectIndices[]{ 0, 1, 2, 3 };
	auto rectMesh = makeMesh<FlowParticleShader>(constView(rectVertices), constView(rectIndices), instancesVbo);

	const i32 circleVertexCount = 50;

	std::vector<Vertex3Pn> vertices;
	std::vector<i32> indices;
	auto coloredShaderMesh = [&vertices, &indices, &instancesVbo]() -> Mesh {
		return makeMesh<ColoredShadingShader>(vertices, indices, instancesVbo);
	};

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
	auto cyllinderMesh = coloredShaderMesh();

	vertices.clear();
	indices.clear();

	for (i32 ui = 0; ui < circleVertexCount; ui++) {
		for (i32 vi = 0; vi < circleVertexCount; vi++) {
			const auto u = f32(ui) / f32(circleVertexCount) * TAU<f32>;
			const auto v = f32(vi) / f32(circleVertexCount - 1) * (PI<f32> / 2.0f);
			const auto pos = Vec3(cos(u) * cos(v), sin(u) * cos(v), sin(v)).normalized();
			const auto& normal = pos;
			vertices.push_back(Vertex3Pn{ pos, normal });
		}
	}

	auto toIndex = [](i32 ui, i32 vi) {
		return ui * circleVertexCount + vi;
	};
	i32 previousUi = circleVertexCount - 1;
	for (i32 ui = 0; ui < circleVertexCount; ui++) {
		i32 previousVi = circleVertexCount - 1;
		for (i32 vi = 0; vi < circleVertexCount; vi++) {
			indicesAddQuad(
				indices,
				toIndex(previousUi, previousVi),
				toIndex(previousUi, vi),
				toIndex(ui, vi),
				toIndex(ui, previousVi)
			);
			previousVi = vi;
		}
		previousUi = ui;
	}
	auto hemisphere = coloredShaderMesh();

	{
		vertices.clear();
		indices.clear();
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
			const auto a = f32(i) / f32(circleVertexCount) * TAU<f32> + centerAngleOffset;
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
		vertices.clear();
		indices.clear();
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

	return Renderer{
		.transform = Mat4::identity,
		.view = Mat4::identity,
		.triangles = TriangleRenderer<Vertex3Pnt>::make<BasicShadingShader>(instancesVbo),
		.trianglesShader = MAKE_GENERATED_SHADER(BASIC_SHADING),
		.coloredTriangles = TriangleRenderer<Vertex3Pnc>::make<ColoredShader>(instancesVbo),
		.coloredShader = MAKE_GENERATED_SHADER(COLORED),
		.flowParticleRectMesh = std::move(rectMesh),
		.flowParticleShader = MAKE_GENERATED_SHADER(FLOW_PARTICLE),
		.coloredShadingShader = MAKE_GENERATED_SHADER(COLORED_SHADING),
		MOVE(cyllinderMesh),
		MOVE(hemisphere),
		MOVE(coneMesh),
		MOVE(circleMesh),
		MOVE(instancesVbo),
		.gfx2d = Gfx2d::make()
	};
}

void Renderer::renderTriangles(f32 opacity) {
	shaderSetUniforms(trianglesShader, BasicShadingFragUniforms{
		.opacity = opacity
	});
	shaderSetUniforms(trianglesShader, BasicShadingVertUniforms{
		.transform = transform
	});
	::renderTriangles(trianglesShader, triangles);
}

void Renderer::renderColoredTriangles(f32 opacity) {
	shaderSetUniforms(coloredShader, ColoredFragUniforms{
		.opacity = opacity
	});
	shaderSetUniforms(coloredShader, ColoredVertUniforms{
		.transform = transform,
		.view = view,
	});
	::renderTriangles(coloredShader, coloredTriangles);
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

void Renderer::drawRectMeshInstances(usize count) {
	glDrawElementsInstanced(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, nullptr, GLsizei(count));
}

void Renderer::flowParticle(f32 size, Vec3 position, Vec4 color) {
	/*const auto model =
		Mat4::translation(position) *
		Mat4(cameraRoatation.inverseIfNormalized().toMatrix()) *
		Mat4(Mat3::scale(Vec3(size))) *
		Mat4::translation(-Vec3(0.5f, 0.5f, 0.0f));*/
	flowParticles.push_back(FlowParticleInstance{
		.positionScale = Vec4(position.x, position.y, position.z, size),
		.color = color,
	});
}

void Renderer::renderFlowParticles(const Mat4& rotateMatrix) {
	flowParticleShader.use();
	shaderSetUniforms(flowParticleShader, FlowParticleVertUniforms{
		.transform = transform,
		.rotate = rotateMatrix
	});
	drawInstances(flowParticleRectMesh.vao, instancesVbo, constView(flowParticles), drawRectMeshInstances);
	flowParticles.clear();
}

void Renderer::initColoredShading() {
	coloredShadingShader.use();
	shaderSetUniforms(coloredShadingShader, ColoredShadingVertUniforms{
		.transform = transform,
		.view = view
	});
}

void Renderer::line(Vec3 a, Vec3 b, f32 radius, Vec3 color, bool caps) {
	const auto length = (b - a).length();
	const auto rotateTranslate = transformMesh(a, b);
	const auto model = rotateTranslate * Mat4(Mat3::scale(Vec3(radius, radius, length)));
	cyllinders.push_back(ColoredShadingInstance{
		.color = color,
		.model = model
	});
	if (caps) {
		const auto hemisphereScale = Mat4(Mat3::scale(Vec3(radius)));
		hemispheres.push_back(ColoredShadingInstance{
			.color = color,
			.model = rotateTranslate * Mat4::translation(Vec3(0.0f, 0.0f, length)) * Mat4(Mat3::scale(radius))
		});
		hemispheres.push_back(ColoredShadingInstance{
			.color = color,
			.model = rotateTranslate * Mat4(Mat3::scale(Vec3(radius, radius, -radius)))
		});
	}
}

void Renderer::renderCyllinders() {
	initColoredShading();
	drawMeshInstances(cyllinderMesh, constView(cyllinders), instancesVbo);
	cyllinders.clear();
}

void Renderer::cyllinder(Vec3 a, Vec3 b, f32 radius, Vec3 color) {
	line(a, b, radius, color, false);
}

void Renderer::sphere(Vec3 center, f32 radius, Vec3 color) {
	const auto translateScale = Mat4::translation(center) * Mat4(Mat3::scale(Vec3(radius, radius, -radius)));
	hemispheres.push_back(ColoredShadingInstance{
		.color = color,
		.model = translateScale
	});
	hemispheres.push_back(ColoredShadingInstance{
		.color = color,
		.model = translateScale * Mat4(Mat3::scale(Vec3(1.0f, 1.0f, -1.0f)))
	});
}

void Renderer::renderHemispheres() {
	initColoredShading();
	drawMeshInstances(hemisphere, constView(hemispheres), instancesVbo);
	hemispheres.clear();
}

void Renderer::cone(Vec3 bottom, Vec3 top, f32 radius, Vec3 color) {
	const auto rotateTranslate = transformMesh(bottom, top);
	cones.push_back(ColoredShadingInstance{
		.color = color,
		.model = rotateTranslate * Mat4(Mat3::scale(Vec3(1.0f, 1.0f, (bottom - top).length())))
	});
}

void Renderer::arrowStartEnd(
	Vec3 start,
	Vec3 end,
	f32 radius,
	f32 coneRadius,
	f32 coneLength,
	Vec3 lineColor,
	Vec3 coneColor) {

	auto dir = end - start;
	const auto length = dir.length();
	dir /= length;

	const auto rotateTranslate = transformMesh(start, end);
	const auto coneScale = Mat4(Mat3::scale(Vec3(coneRadius, coneRadius, coneLength)));

	auto addCone = [&](const Mat4& transform) {
		cones.push_back(ColoredShadingInstance{
			.color = coneColor,
			.model = transform 
		});
		circles.push_back(ColoredShadingInstance{
			.color = coneColor,
			.model = transform 
		});
	};

	if (coneLength < length) {
		circles.push_back(ColoredShadingInstance{
			.color = lineColor,
			.model = rotateTranslate * Mat4(Mat3::scale(radius, radius, 1.0f))
		});
		line(start, end - coneLength * dir, radius, lineColor, false);
		const auto transform = rotateTranslate * Mat4::translation(Vec3(0.0f, 0.0f, length - coneLength)) * coneScale;
		addCone(transform);
	} else {
		addCone(rotateTranslate * coneScale);
	}
}

void Renderer::renderCones() {
	initColoredShading();
	drawMeshInstances(coneMesh, constView(cones), instancesVbo);
	cones.clear();
}

void Renderer::renderCircles() {
	initColoredShading();
	drawMeshInstances(circleMesh, constView(circles), instancesVbo);
	circles.clear();
}

//Mat4 Renderer::viewProjection() {
//	const auto view = camera.viewMatrix();
//	const auto aspectRatio = Window::aspectRatio();
//	const auto projection = Mat4::perspective(PI<f32> / 2.0f, aspectRatio, 0.1f, 1000.0f);
//	return projection * view;
//}
