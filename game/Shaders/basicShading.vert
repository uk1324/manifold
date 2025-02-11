#version 430 core

layout(location = 0) in vec3 vertexPosition; 
layout(location = 1) in vec3 vertexNormal; 

uniform mat4 transform; 

out vec3 normal; 

/*generated end*/

void main() {
	normal = vertexNormal;
	gl_Position = transform * vec4(vertexPosition, 1.0);
}
