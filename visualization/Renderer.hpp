#pragma once

#include <engine/Graphics/ShaderProgram.hpp>
#include <engine/Graphics/Vao.hpp>
#include <engine/Graphics/Ibo.hpp>
#include <game/Shaders/basicShadingData.hpp>
#include <gfx2d/Gfx2d.hpp>
#include <game/Shaders/coloredShadingData.hpp>
#include <game/Shaders/coloredData.hpp>
#include <engine/Math/Quat.hpp>
#include <game/Shaders/flowParticleData.hpp>

void indicesAddTri(std::vector<i32>& indicies, i32 i0, i32 i1, i32 i2);
void indicesAddQuad(std::vector<i32>& indicies, i32 i00, i32 i01, i32 i11, i32 i10);

template<typename Vertex>
struct TriangleRenderer {
	template<typename Shader>
	static TriangleRenderer make(Vbo& instancesVbo);

	i32 currentIndex() const;
	std::vector<i32> indices;
	std::vector<Vertex> vertices;
	Vbo vbo;
	Ibo ibo;
	Vao vao;

	i32 addVertex(const Vertex& vertex);
	void addTri(i32 i0, i32 i1, i32 i2);
	void addQuad(i32 i00, i32 i01, i32 i11, i32 i10);
	void tri(const Vertex& v0, const Vertex& v1, const Vertex3Pnt& v2);
};

struct Renderer {
	static Renderer make();


	Mat4 transform;
	Mat4 view;

	TriangleRenderer<Vertex3Pnt> triangles;
	ShaderProgram& trianglesShader;
	void renderTriangles(f32 opacity);

	TriangleRenderer<Vertex3Pnc> coloredTriangles;
	ShaderProgram& coloredShader;
	void renderColoredTriangles(f32 opacity);

	TriangleRenderer<Vertex3Pn> coloredShadingTriangles;
	void renderColoredShadingTriangles();
	void circleArc(Vec3 center, Vec3 d0, Vec3 d1, f32 radius, Vec3 color);

	struct Mesh {
		Vbo vbo;
		Ibo ibo;
		Vao vao;
		i32 indexCount;
	};
	Mesh flowParticleRectMesh;
	static void drawRectMeshInstances(usize count);

	ShaderProgram& flowParticleShader;
	std::vector<FlowParticleInstance> flowParticles;
	void flowParticle(f32 size, Vec3 position, Vec4 color);
	void renderFlowParticles(const Mat4& rotateMatrix);

	ShaderProgram& coloredShadingShader;
	Mesh cyllinderMesh;
	std::vector<ColoredShadingInstance> cyllinders;
	void initColoredShading();
	// The issue with using this for rendering curves is that the normal vectors are not interpolated because they are the same at both the top and the bottom of the cylinnder, which makes if flat shaded.
	void line(Vec3 a, Vec3 b, f32 radius, Vec3 color, bool caps = true);
	void renderCyllinders();
	void cyllinder(Vec3 a, Vec3 b, f32 radius, Vec3 color);
	Mesh hemisphere;
	std::vector<ColoredShadingInstance> hemispheres;
	void sphere(Vec3 center, f32 radius, Vec3 color);
	void renderHemispheres();
	Mesh coneMesh;
	std::vector<ColoredShadingInstance> cones;
	void cone(Vec3 bottom, Vec3 top, f32 radius, Vec3 color);
	void arrowStartEnd(
		Vec3 start, 
		Vec3 end, 
		f32 radius, 
		f32 coneRadius, 
		f32 coneLength, 
		Vec3 lineColor,
		Vec3 coneColor);
	void arrowStartDirection(
		Vec3 start,
		Vec3 direction,
		f32 radius,
		f32 coneRadius,
		f32 coneLength,
		Vec3 lineColor,
		Vec3 coneColor);
	void renderCones();
	Mesh circleMesh;
	std::vector<ColoredShadingInstance> circles;
	void renderCircles();

	Vbo instancesVbo;

	Gfx2d gfx2d;
};

template<typename Vertex>
template<typename Shader>
TriangleRenderer<Vertex> TriangleRenderer<Vertex>::make(Vbo& instancesVbo) {
	auto vbo = Vbo::generate();
	auto ibo = Ibo::generate();
	auto vao = createInstancingVao<Shader>(vbo, ibo, instancesVbo);
	return TriangleRenderer<Vertex>{
		.vbo = std::move(vbo),
		.ibo = std::move(ibo),
		.vao = std::move(vao),
	};
}

template<typename Vertex>
i32 TriangleRenderer<Vertex>::currentIndex() const {
	return i32(indices.size());
}

template<typename Vertex>
i32 TriangleRenderer<Vertex>::addVertex(const Vertex& vertex) {
	const auto index = vertices.size();
	vertices.push_back(vertex);
	return i32(index);
}

template<typename Vertex>
void TriangleRenderer<Vertex>::addTri(i32 i0, i32 i1, i32 i2) {
	indicesAddTri(indices, i0, i1, i2);
}

template<typename Vertex>
void TriangleRenderer<Vertex>::addQuad(i32 i00, i32 i01, i32 i11, i32 i10) {
	indicesAddQuad(indices, i00, i01, i11, i10);
}

template<typename Vertex>
void TriangleRenderer<Vertex>::tri(const Vertex& v0, const Vertex& v1, const Vertex3Pnt& v2) {
	addTriangle(addVertex(v0), addVertex(v1), addVertex(v2));
}
