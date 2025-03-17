#version 430 core

uniform float opacity; 

in vec2 uv; 

in vec4 color; 
out vec4 fragColor;

/*generated end*/

#include "oit.glsl"

void main() {
	vec2 p = uv;
	p -= 0.5;
	p *= 2.0;
	float d = length(p);
	d = smoothstep(1.0, 0.4, d);
	vec3 col;
	col = mix(color.rgb, vec3(1.0), d);

	fragColor = vec4(col, d * color.a);
	//OIT_OUTPUT(col, d * color.a);
}
