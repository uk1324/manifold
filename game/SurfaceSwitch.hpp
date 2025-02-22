#pragma once

#define SURFACE_SWITCH(valueToSwitchOn, F) \
	switch (valueToSwitchOn) { \
	using enum Surfaces::Type; \
	case TORUS: F(torus); \
	case TREFOIL: F(trefoil); \
	case HELICOID: F(helicoid); \
	case MOBIUS_STRIP: F(mobiusStrip); \
	case PSEUDOSPHERE: F(pseudosphere); \
	case CONE: F(cone); \
	case SPHERE: F(sphere); \
	case PROJECTIVE_PLANE: F(projectivePlane); \
	case KLEIN_BOTTLE: F(kleinBottle); \
	case HYPERBOLIC_PARABOLOID: F(hyperbolicParaboloid); \
	case MONKEY_SADDLE: F(monkeySaddle); \
	case CATENOID: F(catenoid); \
	case ENNEPER_SURFACE: F(enneperSurface); \
	}

	//#define SWITCH_ON_SURFACE(surfaceType, F, ...) \
//switch (surfaceType) { \
//	case TORUS: F(torus, __VA_ARGS__); break; \
//	case TREFOIL: F(trefoil, __VA_ARGS__); break; \
//	case HELICOID: F(helicoid, __VA_ARGS__); break; \
//	case MOBIUS_STRIP: F(mobiusStrip, __VA_ARGS__); break; \
//	case PSEUDOSPHERE: F(pseudosphere, __VA_ARGS__); break; \
//	case CONE: F(cone, __VA_ARGS__); break; \
//	case SPHERE: F(sphere, __VA_ARGS__); break; \
//}
// If the order doesn't match use another macro.