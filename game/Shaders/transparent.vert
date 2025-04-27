#version 430 core

layout(location = 0) in vec3 vertexPosition; 
layout(location = 1) in vec3 vertexNormal; 
layout(location = 2) in mat4 instanceModel; 

uniform mat4 transform; 
uniform mat4 view; 

out vec3 interpolatedNormal; 
out vec4 interpolatedColor; 

/*generated end*/

void main() {

}
