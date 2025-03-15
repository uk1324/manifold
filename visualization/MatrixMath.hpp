#pragma once

template<typename Matrix1, typename Matrix2, typename MatrixOutput>
void matrixMultiply(Matrix1 a, i32 aSizeX, i32 aSizeY, Matrix2 b, i32 bSizeX, i32 bSizeY, MatrixOutput& output) {
	// Matrix multiplicaiton just multiplies each column of rhs by lhs, because of this the output column count is the same as rhs column count.
	// The output dimension of the matrix is it's height so the height of the output is the height of the lhs.
	ASSERT(aSizeX == bSizeY);
	const auto sumIndexMax = aSizeX;
	const auto outputSizeY = aSizeY;
	const auto outputSizeX = bSizeX;

	for (i64 row = 0; row < outputSizeY; row++) {
		for (i64 column = 0; column < outputSizeX; column++) {
			output(column, row) = 0;
			for (i64 i = 0; i < sumIndexMax; i++) {
				output(column, row) += a(i, row) * b(column, i);
			}
		}
	}
}