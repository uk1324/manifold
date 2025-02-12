#pragma once

// The sides u = uMin, v = vMin are fixed
// We then glue the sides u = uMax, v = vMax to them.
// uMin, uMax are parallel
// 
enum class SquareSideConnectivity {
	NONE,
	NORMAL,
	REVERSED
	/*NONE,
	PARALLEL_NORMAL,
	PARALLEL_REVERSED
	PERPENDICULAR_NORMAL,
	PERPENDICULAR_REVERSED*/
};