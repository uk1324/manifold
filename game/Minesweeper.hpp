#pragma once

#include <game/Tiling.hpp>
#include <game/GameRenderer.hpp>
#include <random>

struct Minesweeper {
	Minesweeper();
	void update(GameRenderer& renderer);
	void menuGui();
	void openMenu();
	void closeMenu();

	Tiling t;

	bool isMenuOpen = true;

	std::vector<std::vector<CellIndex>> cellToNeighbours;

	std::vector<bool> isBomb;
	std::vector<i32> neighbouringBombsCount;
	std::vector<bool> isMarked;
	std::vector<bool> isRevealed;

	enum class State {
		BEFORE_FIRST_MOVE,
		GAME_IN_PROGRESS,
		LOST,
		WON,
	};
	State state = State::BEFORE_FIRST_MOVE;

	enum class FirstMoveSetting {
		FIRST_MOVE_NO_BOMB,
		FIRST_MOVE_EMPTY_CELL,
	};
	FirstMoveSetting firstMoveSetting = FirstMoveSetting::FIRST_MOVE_EMPTY_CELL;

	enum class Board {
		CELL_24_SNUB,
		CELL_120,
		SUBDIVIDED_HYPERCUBE,
		CELL_600,
		CELL_600_RECTIFIED,
	};
	static constexpr const char* boardStrings[] = {
		"snub 24 cell",
		"120 cell",
		"subdivided hypercube",
		"600 cell",
		"rectified 600 cell",
	};
	void loadBoard(Board board);
	void loadBoard(const Polytope& polytope);

	Board boardSetting = Board::CELL_120;
	i32 bombCountSetting = 10;

	i32 bombCount = 0;

	bool gameResetButNotStarted = true;

	f32 mouseSensitivity = 1.0f;

	std::optional<i32> highlightNeighbours;

	std::vector<f32> cellHoverAnimationT;

	void initialize();
	void startGame(i32 firstUncoveredCellI);
	void reveal(CellIndex cell);

	void gameOver();

	StereographicCamera stereographicCamera;

	std::random_device dev;
	std::mt19937 rng;
};