#include "Renderer.hpp"
#include <StructUtils.hpp>
#include <engine/Math/Constants.hpp>
#include <gfx/ShaderManager.hpp>
#include <gfx/Instancing.hpp>
#include <engine/Window.hpp>

Renderer Renderer::make() {
	auto instancesVbo = Vbo(1024ull * 10);

	auto trianglesVbo = Vbo::generate();
	auto trianglesIbo = Ibo::generate();
	auto trianglesVao = createInstancingVao<BasicShadingShader>(trianglesVbo, trianglesIbo, instancesVbo);

	return Renderer{
		.viewProjection = Mat4::identity,
		.trianglesShader = MAKE_GENERATED_SHADER(BASIC_SHADING),
		MOVE(trianglesVbo),
		MOVE(trianglesIbo),
		MOVE(trianglesVao),
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

i32 Renderer::addVertex(Vertex3Pn vertex) {
	const auto index = trianglesVertices.size();
	trianglesVertices.push_back(vertex);
	return i32(index);
}

void Renderer::triangle(Vertex3Pn v0, Vertex3Pn v1, Vertex3Pn v2) {
	const auto i0 = addVertex(v0);
	const auto i1 = addVertex(v1);
	const auto i2 = addVertex(v2);
	addTriangle(i0, i1, i2);
}

void Renderer::renderTriangles() {
	trianglesVbo.allocateData(trianglesVertices.data(), trianglesVertices.size() * sizeof(Vertex3Pn));
	trianglesIbo.allocateData(trianglesIndices.data(), trianglesIndices.size() * sizeof(u32));

	shaderSetUniforms(trianglesShader, BasicShadingVertUniforms{
		.transform = viewProjection
	});
	trianglesShader.use();
	trianglesVao.bind();
	glDrawElements(GL_TRIANGLES, trianglesIndices.size(), GL_UNSIGNED_INT, nullptr);

	trianglesVertices.clear();
	trianglesIndices.clear();
}

//Mat4 Renderer::viewProjection() {
//	const auto view = camera.viewMatrix();
//	const auto aspectRatio = Window::aspectRatio();
//	const auto projection = Mat4::perspective(PI<f32> / 2.0f, aspectRatio, 0.1f, 1000.0f);
//	return projection * view;
//}
