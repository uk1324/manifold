#include "GameRenderer.hpp"
#include <gfx/ShaderManager.hpp>
#include <engine/Window.hpp>
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

	auto cubemapMesh = makeMesh<CubemapShader>(constView(cubeMapVertices), constView(cubemapIndices), instancesVbo);

	auto opaqueFbo = Fbo::generate();
	auto opaqueColorTexture = Texture::generate();
	auto depthTexture = Texture::generate();
	{
		opaqueColorTexture.bind();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		depthTexture.bind();
		glBindTexture(GL_TEXTURE_2D, 0);

		opaqueFbo.bind();
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, opaqueColorTexture.handle(), 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture.handle(), 0);
		Fbo::unbind();
	}

	auto revealTexture = Texture::generate();
	auto accumulateTexture = Texture::generate();
	auto transparentFbo = Fbo::generate();
	{

		accumulateTexture.bind(GL_TEXTURE_2D);		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		revealTexture.bind(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		
		transparentFbo.bind();
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, accumulateTexture.handle(), 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, revealTexture.handle(), 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture.handle(), 0);
		const GLenum transparentDrawBuffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, transparentDrawBuffers);
		Fbo::unbind();
	}

	auto gfx2d = Gfx2d::make();


	//const auto icosphere = makeIcosphere(10, 1.0f);
	const auto icosphere = makeIcosphere(30, 1.0f);
	std::vector<SphericalPolygonVertex> icosphereVertices;
	for (i32 i = 0; i < icosphere.positions.size(); i++) {
		icosphereVertices.push_back(SphericalPolygonVertex{
			.position = icosphere.positions[i],
			.normal = icosphere.normals[i]
		});
	}
	auto sphereMesh = makeMesh<SphericalPolygonShader>(constView(icosphereVertices), constView(icosphere.indices), instancesVbo);
	/*auto quadPtVao = Vao::generate();
	quadPtVao.bind();
	createInstancingVao<TexturedFullscreenQuadShader>(gfx2d.quad2dPtVbo, gfx2d.quad2dPtIbo, instancesVbo);
	TexturedFullscreenQuadShader::addAttributesToVao(quadPtVao, gfx2d.quad2dPtVbo, instancesVbo);
	gfx2d.quad2dPtVbo.bind();
	gfx2d.quad2dPtIbo.bind();
	Vao::unbind();
	Ibo::unbind();*/
	auto quadPtVao = createInstancingVao<TexturedFullscreenQuadShader>(gfx2d.quad2dPtVbo, gfx2d.quad2dPtIbo, instancesVbo); 

	GameRenderer renderer{
		MOVE(opaqueFbo),
		MOVE(opaqueColorTexture),
		MOVE(depthTexture),
		MOVE(transparentFbo),
		MOVE(accumulateTexture),
		MOVE(revealTexture),
		.transparencyCompositingShader = MAKE_GENERATED_SHADER(TRANSPARENCY_COMPOSITING),
		MOVE(quadPtVao),
		.fullscreenTexturedQuadShader = MAKE_GENERATED_SHADER(TEXTURED_FULLSCREEN_QUAD),
		.transform = Mat4::identity,
		.view = Mat4::identity,
		.projection = Mat4::identity,
		MOVE(sphereMesh),
		.sphericalPolygonShader = MAKE_GENERATED_SHADER(SPHERICAL_POLYGON),
		MOVE(cubemapMesh),
		.cubemapShader = MAKE_GENERATED_SHADER(CUBEMAP),
		MOVE(bulletRectMesh),
		.bulletShader = MAKE_GENERATED_SHADER(BULLET),
		.coloredShader = MAKE_GENERATED_SHADER(COLORED),
		MOVE(hemisphere),
		MOVE(coneMesh),
		MOVE(circleMesh),
		MOVE(cyllinderMesh),
		MOVE(cubeMesh),
		.surfaceTriangles = TriangleRenderer<Vertex3Pnt>::make<SurfaceShader>(instancesVbo),
		.surfaceShader = MAKE_GENERATED_SHADER(SURFACE),
		.coloredShadingTriangles = TriangleRenderer<Vertex3Pnc>::make<ColoredShadingShader>(instancesVbo),
		.coloredShadingShader = MAKE_GENERATED_SHADER(COLORED_SHADING),
		.homogenousShader = MAKE_GENERATED_SHADER(HOMOGENOUS),
		.infinitePlaneMesh = makeMesh<HomogenousShader>(constView(infinitePlaneVertices), constView(infinitePlaneIndices), instancesVbo),
		MOVE(gfx2d),
		MOVE(instancesVbo),
	};

	renderer.resizeBuffers(Vec2T<i32>(Window::size()));

	return renderer;
}

void GameRenderer::resizeBuffers(Vec2T<i32> screenSize) {
	const auto lastScreenSize = currentScreenSize;
	currentScreenSize = screenSize;
	if (lastScreenSize.has_value() && *lastScreenSize == screenSize) {
		return;
	}
	opaqueColorTexture.bind();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenSize.x, screenSize.y, 0, GL_RGBA, GL_HALF_FLOAT, NULL);
	depthTexture.bind();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, screenSize.x, screenSize.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	accumulateTexture.bind(GL_TEXTURE_2D);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenSize.x, screenSize.y, 0, GL_RGBA, GL_HALF_FLOAT, NULL);
	revealTexture.bind(GL_TEXTURE_2D);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, screenSize.x, screenSize.y, 0, GL_RED, GL_FLOAT, NULL);
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

Vec3 anyPerpendicularVector(Vec3 v) {
	v = v.normalized();
	const auto attempt = cross(v, Vec3(1.0f, 0.0f, 0.0f));
	if (attempt.lengthSquared() == 0.0f) {
		return cross(v, Vec3(0.0f, 1.0f, 0.0f)).normalized();
	}
	return attempt.normalized();
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

void GameRenderer::renderSurfaceTriangles(f32 opacity) {
	shaderSetUniforms(surfaceShader, SurfaceFragUniforms{
		.opacity = opacity
	});
	shaderSetUniforms(surfaceShader, SurfaceVertUniforms{
		.transform = transform
	});
	renderTriangles(surfaceShader, surfaceTriangles);
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
	const auto offset = coloredShadingTriangles.vertices.size();
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
			.cameraPos = cameraPos4
		}
	);
	drawMeshInstances(infinitePlaneMesh, constView(infinitePlanes), instancesVbo);
	infinitePlanes.clear();
}

void GameRenderer::renderSphericalPolygons() {
	sphericalPolygonShader.use();
	shaderSetUniforms(
		sphericalPolygonShader,
		SphericalPolygonVertUniforms{
			.transform = transform
		}
	);
	shaderSetUniforms(
		sphericalPolygonShader,
		SphericalPolygonFragUniforms{
			.cameraPos = cameraPos4,
			.viewInverse4 = viewInverse4
		}
	);

	static bool wireframe = false;
	ImGui::Checkbox("wireframe", &wireframe);
	if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	for (auto& lod : sphereLods) {
		drawInstances(lod.mesh.vao, instancesVbo, constView(lod.instances), [&](usize count) {
			glDrawElementsInstanced(GL_TRIANGLES, lod.mesh.indexCount, GL_UNSIGNED_INT, nullptr, count);
		});
		lod.instances.clear();
	}

	/*drawInstances(sphereMesh.vao, instancesVbo, constView(sphericalPolygonInstances), [&](usize count) {
		glDrawElementsInstanced(GL_TRIANGLES, sphereMesh.indexCount, GL_UNSIGNED_INT, nullptr, count);
	});
	sphericalPolygonInstances.clear();*/
	if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void GameRenderer::renderSphericalPolygon(f32 radius, Mat4 transform, Vec4 n0, Vec4 n1, Vec4 n2, Vec4 planeNormal) {
	const auto instance = SphericalPolygonInstance{
		.transform = transform,
		.n0 = n0,
		.n1 = n1,
		.n2 = n2,
		.planeNormal = planeNormal
	};

	i32 lodLevelToUse = sphereLods.size() - 1;
	for (; lodLevelToUse > 0; lodLevelToUse--) {
		if (radius > sphereLods[lodLevelToUse].setting.minRadius) {
			//lodLevelToUse--;
			break;
		}
	}
	ImGui::Text("lod level: %d, ", lodLevelToUse);
	ImGui::Text("lod radius %g", sphereLods[lodLevelToUse].setting.minRadius);
	ImGui::Text("radius %g", radius);
	auto& lod = sphereLods[lodLevelToUse];
	ImGui::Text("triangle count %d", (lod.mesh.indexCount) / 3);
	lod.instances.push_back(instance);
}

void GameRenderer::generateSphereLods(const std::vector<SphereLodSetting>& settings) {
	sphereLods.clear();
	// The bigger the sphere the closer it gets to passing though 0. So to limit the number of vertices drawn this will generate the sphere at 0 with radius 1. Then scale it and translate it so that a point passes though 0 and then discard vertices that are too far away.
	for (const auto& setting : settings) {
		auto sphere = makeIcosphere(setting.divisionCount, 1.0f);
		//std::vector<i32> verticesToDiscard;
		const auto VERTEX_REMOVED = -1;
		std::vector<i32> oldIndexToNewIndex;
		i32 newIndexCount = 0;
		for (i32 i = 0; i < sphere.positions.size(); i++) {
			const auto v = sphere.positions[i];
			// sphereLodCenter gets mapped to 0.
			const auto transformedV = v * setting.minRadius - sphereLodCenter.normalized() * setting.minRadius;
			const auto fogFar = 20.0f;
			//const auto r = fogFar + 10.0f;
			const auto r = fogFar + 2.0f;
			if (Vec3(0.0f).distanceTo(transformedV) > r) {
				oldIndexToNewIndex.push_back(VERTEX_REMOVED);
			} else {
				oldIndexToNewIndex.push_back(newIndexCount);
				newIndexCount++;
			}
		}
		//MeshTriPn mesh;
		std::vector<SphericalPolygonVertex> vertices;
		std::vector<i32> indices;
		for (i32 i = 0; i < sphere.positions.size(); i++) {
			if (oldIndexToNewIndex[i] == VERTEX_REMOVED) {
				continue;
			}
			vertices.push_back(SphericalPolygonVertex{
				.position = sphere.positions[i],
				.normal = sphere.normals[i]
			});
		}
		for (i32 i = 0; i < sphere.indices.size(); i += 3) {
			bool faceRemoved = false;
			for (i32 j = 0; j < 3; j++) {
				if (oldIndexToNewIndex[sphere.indices[i + j]] == VERTEX_REMOVED) {
					faceRemoved = true;
					break;
				}
			}
			if (faceRemoved) {
				continue;
			}
			for (i32 j = 0; j < 3; j++) {
				const auto index = oldIndexToNewIndex[sphere.indices[i + j]];
				ASSERT(index != VERTEX_REMOVED);
				indices.push_back(index);
			}
		}
		sphereLods.push_back(SphereLodLevel{
			.setting = setting,
			.mesh = makeMesh<SphericalPolygonShader>(constView(vertices), constView(indices), instancesVbo),
		});
	}

}
