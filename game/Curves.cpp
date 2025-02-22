#include "Curves.hpp"
#include <Assertions.hpp>

#define CURVE_SWITCH(valueToSwitchOn, F) \
	switch (valueToSwitchOn) { \
		using enum Curves::Type; \
		case HELIX: F(helix) \
	} \

Vec3 Curves::position(f32 t) const {
	#define R(name) return name.position(t);
	CURVE_SWITCH(type, R);
	#undef R
	ASSERT_NOT_REACHED();
}

Vec3 Curves::tangent(f32 t) const {
	#define R(name) return name.tangent(t);
	CURVE_SWITCH(type, R);
	#undef R
	ASSERT_NOT_REACHED();
}

Vec3 Curves::normal(f32 t) const {
	#define R(name) return name.normal(t);
	CURVE_SWITCH(type, R);
	#undef R
	ASSERT_NOT_REACHED();
}

Vec3 Curves::binormal(f32 t) const {
	#define R(name) return name.binormal(t);
	CURVE_SWITCH(type, R);
	#undef R
	ASSERT_NOT_REACHED();
}

f32 Curves::curvature(f32 t) const {
	#define R(name) return name.curvature(t);
	CURVE_SWITCH(type, R);
	#undef R
	ASSERT_NOT_REACHED();
}

f32 Curves::torsion(f32 t) const {
	#define R(name) return name.torsion(t);
	CURVE_SWITCH(type, R);
	#undef R
	ASSERT_NOT_REACHED();	
}
