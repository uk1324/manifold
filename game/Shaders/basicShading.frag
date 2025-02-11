#version 430 core

in vec3 normal; 
out vec4 fragColor;

/*generated end*/

void main() {
	vec3 lightDir = vec3(0, -1, 0);
	float diffuse = max(0.0, dot(normal, -lightDir)) + 0.1;
	fragColor = vec4(vec3(diffuse), 1.0);
}
