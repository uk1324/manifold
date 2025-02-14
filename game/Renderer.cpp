#include "Renderer.hpp"
#include <StructUtils.hpp>
#include <engine/Math/Constants.hpp>
#include <engine/Math/Interpolation.hpp>
#include <gfx/ShaderManager.hpp>
#include <gfx/Instancing.hpp>
#include <engine/Window.hpp>

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
	indicies.push_back(i00);
	indicies.push_back(i10);
	indicies.push_back(i11);

	indicies.push_back(i00);
	indicies.push_back(i11);
	indicies.push_back(i01);
}

Renderer Renderer::make() {
	auto instancesVbo = Vbo(1024ull * 10);

	auto trianglesVbo = Vbo::generate();
	auto trianglesIbo = Ibo::generate();
	auto trianglesVao = createInstancingVao<BasicShadingShader>(trianglesVbo, trianglesIbo, instancesVbo);

	std::vector<Vertex3Pn> vertices;
	std::vector<i32> indices;
	const i32 circleVertexCount = 50;

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
	auto cyllinderVbo = Vbo(vertices.data(), vertices.size() * sizeof(Vertex3Pn));
	auto cyllinderIbo = Ibo(indices.data(), indices.size() * sizeof(i32));
	auto cyllinderVao = createInstancingVao<ColoredShadingShader>(cyllinderVbo, cyllinderIbo, instancesVbo);
	const auto cyllinderIndexCount = i32(indices.size());

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
	auto hemisphereVbo = Vbo(vertices.data(), vertices.size() * sizeof(Vertex3Pn));
	auto hemisphereIbo = Ibo(indices.data(), indices.size() * sizeof(i32));
	auto hemisphereVao = createInstancingVao<ColoredShadingShader>(hemisphereVbo, hemisphereIbo, instancesVbo);
	const auto hemisphereIndexCount = i32(indices.size());

	return Renderer{
		.transform = Mat4::identity,
		.view = Mat4::identity,
		.trianglesShader = MAKE_GENERATED_SHADER(BASIC_SHADING),
		MOVE(trianglesVbo),
		MOVE(trianglesIbo),
		MOVE(trianglesVao),
		.coloredShader = MAKE_GENERATED_SHADER(COLORED_SHADING),
		MOVE(cyllinderVao),
		MOVE(cyllinderVbo),
		MOVE(cyllinderIbo),
		.cyllinderIndexCount = cyllinderIndexCount,
		MOVE(hemisphereVao),
		MOVE(hemisphereVbo),
		MOVE(hemisphereIbo),
		.hemisphereIndexCount = hemisphereIndexCount,
		MOVE(instancesVbo),
	};
}

void Renderer::addTriangle(i32 i0, i32 i1, i32 i2) {
	trianglesIndices.push_back(i0);
	trianglesIndices.push_back(i1);
	trianglesIndices.push_back(i2);
}

void Renderer::addQuad(i32 i0, i32 i1, i32 i2, i32 i3) {
	/*
	i3-i2
	| / |
	i0-i1
	*/
	trianglesIndices.push_back(i0);
	trianglesIndices.push_back(i1);
	trianglesIndices.push_back(i2);

	trianglesIndices.push_back(i0);
	trianglesIndices.push_back(i2);
	trianglesIndices.push_back(i3);
}

i32 Renderer::addVertex(Vertex3Pnt vertex) {
	const auto index = trianglesVertices.size();
	trianglesVertices.push_back(vertex);
	return i32(index);
}

void Renderer::triangle(Vertex3Pnt v0, Vertex3Pnt v1, Vertex3Pnt v2) {
	const auto i0 = addVertex(v0);
	const auto i1 = addVertex(v1);
	const auto i2 = addVertex(v2);
	addTriangle(i0, i1, i2);
}

void Renderer::renderTriangles(f32 opacity) {
	trianglesVbo.allocateData(trianglesVertices.data(), trianglesVertices.size() * sizeof(Vertex3Pnt));
	trianglesIbo.allocateData(trianglesIndices.data(), trianglesIndices.size() * sizeof(u32));

	shaderSetUniforms(trianglesShader, BasicShadingFragUniforms{
		.opacity = opacity
	});
	shaderSetUniforms(trianglesShader, BasicShadingVertUniforms{
		.transform = transform
	});
	trianglesShader.use();
	trianglesVao.bind();
	glDrawElements(GL_TRIANGLES, trianglesIndices.size(), GL_UNSIGNED_INT, nullptr);

	trianglesVertices.clear();
	trianglesIndices.clear();
}

void Renderer::line(Vec3 a, Vec3 b, f32 radius, Vec3 color) {
	auto v0 = b - a;
	const auto length = v0.length();
	v0 /= length;
	const auto v1 = anyPerpendicularVector(v0);
	const auto v2 = cross(v0, v1).normalized();
	const auto rotateTranslate = Mat4::translation(a) * Mat4(Mat3(v1, v2, v0));
	const auto model = rotateTranslate * Mat4(Mat3::scale(Vec3(radius, radius, length)));
	cyllinders.push_back(ColoredShadingInstance{
		.color = color,
		.model = model
	});
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

void Renderer::renderCyllinders() {
	coloredShader.use();
	shaderSetUniforms(coloredShader, ColoredShadingVertUniforms{
		.transform = transform,
		.view = view
	});
	drawInstances(cyllinderVao, instancesVbo, constView(cyllinders), [&](usize count) {
		glDrawElementsInstanced(GL_TRIANGLES, cyllinderIndexCount, GL_UNSIGNED_INT, nullptr, count);
	});
	cyllinders.clear();
}

void Renderer::renderHemispheres() {
	coloredShader.use();
	shaderSetUniforms(coloredShader, ColoredShadingVertUniforms{
		.transform = transform,
		.view = view
	});
	drawInstances(hemisphereVao, instancesVbo, constView(hemispheres), [&](usize count) {
		glDrawElementsInstanced(GL_TRIANGLES, hemisphereIndexCount, GL_UNSIGNED_INT, nullptr, count);
	});
	hemispheres.clear();
}

//Mat4 Renderer::viewProjection() {
//	const auto view = camera.viewMatrix();
//	const auto aspectRatio = Window::aspectRatio();
//	const auto projection = Mat4::perspective(PI<f32> / 2.0f, aspectRatio, 0.1f, 1000.0f);
//	return projection * view;
//}
