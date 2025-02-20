#include <game/Surfaces.hpp>

#define SWITCH() \
	switch (selected) { \
	using enum Type; \
		case TORUS: R(torus); \
		case TREFOIL: R(trefoil); \
		case HELICOID: R(helicoid); \
		case MOBIUS_STRIP: R(mobiusStrip); \
		case PSEUDOSPHERE: R(pseudosphere); \
		case CONE: R(cone); \
		case SPHERE: R(sphere); \
		case PROJECTIVE_PLANE: R(sphere); \
	}

Vec3 Surfaces::position(Vec2 uv) const {
	#define R(name) return name.position(uv.x, uv.y)
	SWITCH();
	#undef R
	ASSERT_NOT_REACHED();
}

Vec3 Surfaces::normal(Vec2 uv) const {
	#define R(name) return name.normal(uv.x, uv.y)
	SWITCH();
	#undef R
	ASSERT_NOT_REACHED();
}

Vec3 Surfaces::tangentU(Vec2 uv) const {
	#define R(name) return name.tangentU(uv.x, uv.y)
	SWITCH();
	#undef R
	ASSERT_NOT_REACHED();
}

Vec3 Surfaces::tangentV(Vec2 uv) const {
	#define R(name) return name.tangentV(uv.x, uv.y)
	SWITCH();
	#undef R
	ASSERT_NOT_REACHED();
}

PrincipalCurvatures Surfaces::principalCurvatures(Vec2 uv) const {
	#define R(name) return name.principalCurvatures(uv.x, uv.y)
	SWITCH();
	#undef R
	ASSERT_NOT_REACHED();
}

const char* surfaceNameStr(Surfaces::Type surface) {
	switch (surface) {
		using enum Surfaces::Type;
	case TORUS: return "torus";
	case TREFOIL: return "trefoil";
	case HELICOID: return "helicoid";
	case MOBIUS_STRIP: return "mobius strip";
	case PSEUDOSPHERE: return "pseudosphere";
	case CONE: return "cone";
	case SPHERE: return "sphere";
	case PROJECTIVE_PLANE: return "projective plane";
	}
	return "";
}
