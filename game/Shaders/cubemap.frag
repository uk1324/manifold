#version 430 core

uniform vec3 directionalLightDirection; 
uniform float time; 

in vec3 unnormalizedDirection; 
out vec4 fragColor;

/*generated end*/

vec3 sampleSkybox(vec3 v) {
	return (v + 1.0) / 2.0 * 0.2;
}

void main() {
	vec3 direction = normalize(unnormalizedDirection);
	vec3 color = sampleSkybox(direction);
	fragColor = vec4(color, 1.0);
}
