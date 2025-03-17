
layout(location = 0) out vec4 accum;
layout(location = 1) out float reveal;

#define OIT_OUTPUT(color, alpha) { \
		float weight = clamp(pow(min(1.0, alpha * 10.0) + 0.01, 3.0) * 1e8 * pow(1.0 - gl_FragCoord.z * 0.9, 3.0), 1e-2, 3e3); \
		accum = vec4(color.rgb * alpha, alpha) * weight; \
		reveal = alpha; \
	}
