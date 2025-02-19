#version 430 core

uniform float opacity; 

in vec3 interpolatedNormal; 
in vec3 interpolatedColor; 
out vec4 fragColor;

/*generated end*/

void main() {
	vec3 col = interpolatedColor;
	fragColor = vec4(col, opacity);
}
