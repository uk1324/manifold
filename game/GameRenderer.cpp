#include "GameRenderer.hpp"
#include <gfx/ShaderManager.hpp>
#include <StructUtils.hpp>
#include <engine/Math/Constants.hpp>

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

GameRenderer GameRenderer::make() {
	auto instancesVbo = Vbo(1024ull * 10);

	std::vector<Vertex3Pn> vertices;
	std::vector<i32> indices;
	auto coloredShaderMesh = [&vertices, &indices, &instancesVbo]() -> Mesh {
		return makeMesh<ColoredShader>(constView(vertices), constView(indices), instancesVbo);
	};

	const i32 circleVertexCount = 50;

	{
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
	}
	auto hemisphere = coloredShaderMesh();

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
	auto bulletRectMesh = makeMesh<BulletShader>(constView(rectVertices), constView(rectIndices), instancesVbo);

	auto cubemapMesh = makeMesh<CubemapShader>(constView(cubeVertices), constView(cubeIndices), instancesVbo);

	return GameRenderer{
		.transform = Mat4::identity,
		.view = Mat4::identity,
		.projection = Mat4::identity,
		MOVE(cubemapMesh),
		.cubemapShader = MAKE_GENERATED_SHADER(CUBEMAP),
		MOVE(bulletRectMesh),
		.bulletShader = MAKE_GENERATED_SHADER(BULLET),
		.coloredShader = MAKE_GENERATED_SHADER(COLORED),
		MOVE(hemisphere),
		.surfaceTriangles = TriangleRenderer<Vertex3Pnt>::make<SurfaceShader>(instancesVbo),
		.surfaceShader = MAKE_GENERATED_SHADER(SURFACE),
		MOVE(instancesVbo)
		//.gfx2d = Gfx2d::make()
	};
}

void GameRenderer::renderCubemap() {
	//shaderSetUniforms(cubemapShader, CubemapVertUniforms{
	//	/*.transform = view.removedTranslation() * projection,*/
	//	.transform = projection * view.removedTranslation(),
	//});
	shaderSetUniforms(cubemapShader, CubemapVertUniforms{
		/*.transform = view.removedTranslation() * projection,*/
		.transform = projection * view.removedTranslation()
	});
	CubemapFragUniforms cubemapShaderFragUniforms;
	/*cubemapShaderFragUniforms.directionalLightDirection = directionalLightDirection;
	cubemapShaderFragUniforms.time = elapsed;*/
	shaderSetUniforms(cubemapShader, cubemapShaderFragUniforms);
	CubemapInstance instance{};

	cubemapShader.use();
	glDepthMask(GL_FALSE);
	drawMeshInstances(cubemapMesh, constView(instance), instancesVbo);
	glDepthMask(GL_TRUE);
}

void GameRenderer::drawRectMeshInstances(usize count) {
	glDrawElementsInstanced(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, nullptr, GLsizei(count));
}

void GameRenderer::bullet(f32 size, Vec3 position, Vec4 color) {
	bulletInstances.push_back(BulletInstance{
		.positionScale = Vec4(position.x, position.y, position.z, size),
		.color = color,
	});
}

void GameRenderer::renderBullets(const Mat4& rotateMatrix) {
	bulletShader.use();
	shaderSetUniforms(bulletShader, BulletVertUniforms{
		.transform = transform,
		.rotate = rotateMatrix
	});
	drawInstances(bulletRectMesh.vao, instancesVbo, constView(bulletInstances), drawRectMeshInstances);
	bulletInstances.clear();
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

void GameRenderer::renderSurfaceTriangles(f32 opacity) {
	shaderSetUniforms(surfaceShader, SurfaceFragUniforms{
		.opacity = opacity
		});
	shaderSetUniforms(surfaceShader, SurfaceVertUniforms{
		.transform = transform
	});
	renderTriangles(surfaceShader, surfaceTriangles);
}