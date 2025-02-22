#pragma once

#include <game/Renderer.hpp>
#include <game/SurfaceInfo.hpp>
#include <game/FpsCamera3d.hpp>
#include <game/SurfaceCamera.hpp>
#include <Array2d.hpp>
#include <game/Surfaces.hpp>
#include <game/SurfaceInfo.hpp>
#include <game/GeodesicTool.hpp>
#include <game/VectorFieldTool.hpp>
#include <game/CurvatureTool.hpp>
#include <game/Visualization4d.hpp>

struct SurfaceVisualization {
	SurfaceVisualization();
	void update(Renderer& renderer);
	
	void gui();
	bool showGui = true;

	void initializeSelectedSurface();

	void calculateIntersections(Vec3 cameraPosition, Vec3 cameraForward);
	std::vector<MeshIntersection> intersections;

	GeodesicTool geodesicTool;

	FpsCamera3d fpsCamera;
	SufaceCamera surfaceCamera;
	struct SurfaceCameraUpdateResult {
		Mat4 view;
		Vec3 cameraPosition;
	};
	SurfaceCameraUpdateResult updateSurfaceCamera(f32 dt);

	CurvatureTool curvatureTool;

	VectorFieldTool vectorFieldTool;

	f32 transitionT = 1.0f;

	f32 meshOpacity = 0.5f;
	enum class MeshRenderMode {
		GRID,
		CURVATURE
	} meshRenderMode = MeshRenderMode::GRID;

	SurfaceData surfaceData;

	enum class CameraMode {
		ON_SURFACE,
		IN_SPACE
	} cameraMode = CameraMode::IN_SPACE;

	Surfaces surfaces;

	enum class ToolType {
		NONE,
		GEODESICS,
		CURVATURE,
		FLOW,
	} selectedTool = ToolType::NONE;
};