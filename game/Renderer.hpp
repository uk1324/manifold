#pragma once

#include <engine/Graphics/ShaderProgram.hpp>
#include <engine/Graphics/Vao.hpp>
#include <engine/Graphics/Ibo.hpp>
#include <game/Shaders/basicShadingData.hpp>
#include <game/Shaders/coloredShadingData.hpp>


void indicesAddTri(std::vector<i32>& indicies, i32 i0, i32 i1, i32 i2);
void indicesAddQuad(std::vector<i32>& indicies, i32 i00, i32 i01, i32 i11, i32 i10);

struct Renderer {
	static Renderer make();

	void addTriangle(i32 i0, i32 i1, i32 i2);
	void addQuad(i32 i0, i32 i1, i32 i2, i32 i3);
	i32 addVertex(Vertex3Pnt vertex);
	void triangle(Vertex3Pnt v0, Vertex3Pnt v1, Vertex3Pnt v2);
	void renderTriangles(f32 opacity);

	Mat4 transform;
	Mat4 view;

	std::vector<i32> trianglesIndices;
	std::vector<Vertex3Pnt> trianglesVertices;
	ShaderProgram& trianglesShader;
	Vbo trianglesVbo;
	Ibo trianglesIbo;
	Vao trianglesVao;

	ShaderProgram& coloredShader;
	Vao cyllinderVao;
	Vbo cyllinderVbo;
	Ibo cyllinderIbo;
	i32 cyllinderIndexCount;
	std::vector<ColoredShadingInstance> cyllinders;
	// The issue with using this for rendering curves is that the normal vectors are not interpolated because they are the same at both the top and the bottom of the cylinnder, which makes if flat shaded.
	void line(Vec3 a, Vec3 b, f32 radius, Vec3 color);
	void renderCyllinders();
	Vao hemisphereVao;
	Vbo hemisphereVbo;
	Ibo hemisphereIbo;
	i32 hemisphereIndexCount;
	std::vector<ColoredShadingInstance> hemispheres;
	void renderHemispheres();

	Vbo instancesVbo;

};