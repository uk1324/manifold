#version 430 core

layout(location = 0) in vec3 vertexPosition; 
layout(location = 1) in vec3 vertexNormal; 
layout(location = 2) in vec3 vertexColor; 

uniform mat4 transform; 
uniform mat4 view; 

out vec3 interpolatedNormal; 
out vec3 interpolatedColor; 

/*generated end*/

void main() {
	interpolatedNormal = vertexNormal;
	interpolatedColor = vertexColor;
	gl_Position = transform * vec4(vertexPosition, 1.0);
}
