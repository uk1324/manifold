#include "GameRenderer.hpp"
#include <gfx/ShaderManager.hpp>
#include <StructUtils.hpp>

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

GameRenderer GameRenderer::make() {
	auto instancesVbo = Vbo(1024ull * 10);

	return GameRenderer{
		.transform = Mat4::identity,
		.view = Mat4::identity,
		.surfaceTriangles = TriangleRenderer<Vertex3Pnt>::make<SurfaceShader>(instancesVbo),
		.surfaceShader = MAKE_GENERATED_SHADER(SURFACE),
		MOVE(instancesVbo)
		//.gfx2d = Gfx2d::make()
	};
}

void GameRenderer::renderSurfaceTriangles(f32 opacity) {
	shaderSetUniforms(surfaceShader, SurfaceFragUniforms{
		.opacity = opacity
		});
	shaderSetUniforms(surfaceShader, SurfaceVertUniforms{
		.transform = transform
	});
	renderTriangles(surfaceShader, surfaceTriangles);
}