#version 430 core

in vec2 position; 
out vec4 fragColor;

/*generated end*/

uniform sampler2D textureSampler;

void main() {
	fragColor = vec4(texture(textureSampler, position).rgb, 1.0);
	//fragColor = vec4(vec3(1.0, 0.0, 0.0), 1.0);
}
