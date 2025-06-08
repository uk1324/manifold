#pragma once

#include <game/Tiling.hpp>
#include <game/GameRenderer.hpp>
#include <random>

struct Minesweeper {
	Minesweeper();
	void update(GameRenderer& renderer);

	Tiling t;

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

	bool gameResetButNotStarted = true;

	std::vector<f32> cellHoverAnimationT;

	void initialize();
	void startGame(i32 firstUncoveredCellI);
	void reveal(CellIndex cell);

	void gameOver();

	StereographicCamera stereographicCamera;

	std::random_device dev;
	std::mt19937 rng;
};