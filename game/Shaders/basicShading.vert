#version 430 core

layout(location = 0) in vec3 vertexPosition; 
layout(location = 1) in vec3 vertexNormal; 
layout(location = 2) in vec2 vertexUv; 

uniform mat4 transform; 

out vec3 interpolatedNormal; 
out vec2 uv; 

/*generated end*/

void main() {
	uv = vertexUv;
	interpolatedNormal = vertexNormal;
	gl_Position = transform * vec4(vertexPosition, 1.0);
}
