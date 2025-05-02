#pragma once

#include <engine/Graphics/Fbo.hpp>
#include <engine/gfx2d/Gfx2d.hpp>
#include <game/TriangleRenderer.hpp>
#include <game/Shaders/surfaceData.hpp>
#include <game/Shaders/coloredData.hpp>
#include <game/Shaders/bulletData.hpp>
#include <game/Shaders/coloredShadingData.hpp>
#include <game/LineGenerator.hpp>
#include <game/Shaders/sphericalPolygonData.hpp>
#include <game/Shaders/homogenousData.hpp>
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

	void resizeBuffers(Vec2T<i32> screenSize);

	std::optional<Vec2T<i32>> currentScreenSize;

	Fbo opaqueFbo;
	Texture opaqueColorTexture;
	Texture depthTexture;

	Fbo transparentFbo;
	Texture accumulateTexture;
	static constexpr i32 accumulateTextureColorBufferIndex = 0;
	Texture revealTexture;
	static constexpr i32 revealTextureColorBufferIndex = 1;

	ShaderProgram& transparencyCompositingShader;
	Vao quadPtVao;

	ShaderProgram& fullscreenTexturedQuadShader;

	Mat4 transform;
	Mat4 view;
	Mat4 projection;
	Vec3 cameraForward;
	Vec3 cameraPosition;

	
	Mesh sphereMesh;
	ShaderProgram& sphericalPolygonShader;

	Mesh cubemapMesh;
	ShaderProgram& cubemapShader;
	void renderCubemap();

	Mesh bulletRectMesh;
	static void drawRectMeshInstances(usize count);

	ShaderProgram& bulletShader;
	std::vector<BulletInstance> bulletInstances;
	void bullet(f32 size, Vec3 position, Vec4 color);
	void renderBullets(const Mat4& rotateMatrix);

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

	TriangleRenderer<Vertex3Pnt> surfaceTriangles;
	ShaderProgram& surfaceShader;
	void renderSurfaceTriangles(f32 opacity);

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

	std::vector<SphericalPolygonInstance> sphericalPolygonInstances;
	void renderSphericalPolygons();
	void renderSphericalPolygon(f32 radius, Mat4 transform, Vec4 n0, Vec4 n1, Vec4 n2, Vec4 planeNormal);

	struct SphereLodSetting {
		f32 minRadius;
		i32 divisionCount;
	};
	Vec3 sphereLodCenter = icosahedronVertices[0];
	void generateSphereLods(const std::vector<SphereLodSetting>& settings);
	struct SphereLodLevel {
		SphereLodSetting setting;
		Mesh mesh;
		std::vector<SphericalPolygonInstance> instances;
	};
	std::vector<SphereLodLevel> sphereLods;

	Gfx2d gfx2d;

	Vec4 cameraPos4 = Vec4(0.0f);
	Mat4 viewInverse4 = Mat4::identity;

	Vbo instancesVbo;
};