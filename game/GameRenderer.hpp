#pragma once

#include <engine/Graphics/Fbo.hpp>
#include <engine/gfx2d/Gfx2d.hpp>
#include <game/TriangleRenderer.hpp>
#include <game/Shaders/coloredData.hpp>
#include <game/Shaders/coloredShadingData.hpp>
#include <game/LineGenerator.hpp>
#include <game/Shaders/homogenousData.hpp>
#include <game/Shaders/sphereImpostorData.hpp>
#include <game/Shaders/sphereImpostor2Data.hpp>
#include <game/Polyhedra.hpp>
#include <game/Cubemap.hpp>

struct Mesh {
	Vbo vbo;
	Ibo ibo;
	Vao vao;
	i32 indexCount;
};

struct GameRenderer {
	static GameRenderer make();

	Mat4 transform;
	Mat4 view;
	Mat4 projection;
	Vec3 cameraForward;
	Vec3 cameraPosition;
	
	void initColoredShader();
	ShaderProgram& coloredShader;

	Mesh hemisphere;
	std::vector<ColoredInstance> hemispheres;
	void sphere(Vec3 center, f32 radius, Vec3 color);
	void renderHemispheres();

	Mesh coneMesh;
	std::vector<ColoredInstance> cones;
	void cone(Vec3 bottom, Vec3 top, f32 radius, Vec3 color);
	void renderCones();

	Mesh circleMesh;
	std::vector<ColoredInstance> circles;
	void renderCircles();

	Mesh cyllinderMesh;
	std::vector<ColoredInstance> cyllinders;
	void renderCyllinders();

	Mesh cubeMesh;
	std::vector<ColoredInstance> cubes;
	void cube(Vec3 color);
	void renderCubes();

	void line(Vec3 a, Vec3 b, f32 radius, Vec3 color, bool caps = true);

	TriangleRenderer<Vertex3Pnc> coloredShadingTriangles;
	ShaderProgram& coloredShadingShader;
	Mat4 coloredShadingModel = Mat4::identity;
	void renderColoredShadingTriangles(const ColoredShadingInstance& instance);
	void coloredShadingTrianglesAddMesh(const std::vector<Vec3>& positions, const std::vector<Vec3>& normals, const std::vector<i32>& indices, Vec3 color);
	void coloredShadingTrianglesAddMesh(const LineGenerator& lineGenerator, Vec3 color);

	ShaderProgram& homogenousShader;
	Mesh infinitePlaneMesh;
	std::vector<HomogenousInstance> infinitePlanes;
	void renderInfinitePlanes();

	ShaderProgram& sphereImpostorsShader;
	ShaderProgram& sphereImpostors2Shader;
	Mesh sphereImpostorMesh;
	Mesh sphereImpostorMeshTri;
	std::vector<SphereImpostorInstance> sphereImpostors;
	std::vector<SphereImpostor2Instance> sphereImpostorsCubes;
	void sphereImpostor(Mat4 transform, Vec3 position, f32 radius, Vec4 n0, Vec4 n1, Vec4 n2, Vec4 n3, Vec4 n4, Vec4 planeNormal);
	void sphereImpostorCube(Vec4 center4, Mat4 transform, Vec3 position, f32 radius, Vec4 n0, Vec4 n1, Vec4 n2, Vec4 n3, Vec4 planeNormal);
	void renderSphereImpostors();

	bool useImpostorsTriangles = true;

	void stereographicLineSegment(Vec4 e0, Vec4 e1);

	void planeTriangle(const Plane& plane, Vec4 edgeNormal0, Vec4 edgeNormal1, Vec4 edgeNormal2, Vec4 edgeNormal3, Vec4 edgeNormal4, Vec4 planeNormal);
	void sphericalTriangle(Vec3 sp0, Vec3 sp1, Vec3 sp2, const Sphere& sphere, Vec4 n0, Vec4 n1, Vec4 n2, Vec4 n3, Vec4 n4, Vec4 planeNormal);
	void stereographicTriangle(Vec4 p0, Vec4 p1, Vec4 p2, Vec4 planeNormal4, Vec4 edgeNormal0, Vec4 edgeNormal1, Vec4 edgeNormal2, Vec4 edgeNormal3, Vec4 edgeNormal4);

	void stereographicSphere(Vec4 pos, f32 radius);

	LineGenerator lineGenerator;

	Gfx2d gfx2d;

	Vec4 cameraPos4 = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
	Mat4 viewInverse4 = Mat4::identity;

	Vbo instancesVbo;

};