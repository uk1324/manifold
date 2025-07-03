//#pragma once
//
//#include <game/GameRenderer.hpp>
//#include <game/FpsCamera3d.hpp>
//#include <game/StereographicCamera.hpp>
//#include <game/Physics/World.hpp>
//#include <game/Tiling.hpp>
//
//struct Game {
//	static Game make();
//
//	void update(GameRenderer& renderer);
//
//	Tiling t;
//	std::vector<bool> isCellSet;
//
//	std::vector<std::vector<i32>> cellToNeighbouringCells;
//
//
//	enum class CameraType {
//		NORMAL,
//		STEREOGRAPHIC,
//	} selectedCamera = CameraType::STEREOGRAPHIC;
//
//	enum class Tool {
//		BUILDING,
//		PUSHING
//	};
//	Tool tool = Tool::BUILDING;
//
//	World world;
//	bool cellsModified = false;
//	struct CellBody {
//		i32 faceIndex;
//		BodyId bodyId;
//	};
//	std::vector<CellBody> cellsBodies;
//	void updateCellsBodies();
//
//	bool updateGameOfLife = false;
//	void gameOfLifeStep();
//	i32 frame = 0;
//
//	bool randomizeSeed = true;
//	i32 noiseSeed = 0;
//	f32 noiseScale = 1.0f;
//	f32 noiseGain = 0.5f;
//	f32 noiseLacunarity = 2.0f;
//	void terrainGenerationGui();
//	void generateTerrain();
//
//	FpsCamera3d camera;
//	StereographicCamera stereographicCamera;
//};