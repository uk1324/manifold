#version 430 core

uniform float opacity; 

in vec2 uv; 

in vec4 color; 
out vec4 fragColor;

/*generated end*/

void main() {
	vec2 p = uv;
	p -= 0.5;
	p *= 2.0;
	
	float d = length(p);
	float smoothing = fwidth(d) * 2.0;
	d -= 1.0 - smoothing;
	d = smoothstep(smoothing, 0.0 , d);
	fragColor = vec4(color.rgb, d * color.a);
}
