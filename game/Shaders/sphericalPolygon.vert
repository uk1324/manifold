#version 430 core

layout(location = 0) in vec3 vertexPosition; 
layout(location = 1) in vec3 vertexNormal; 
layout(location = 2) in mat4 instanceTransform; 
layout(location = 6) in vec4 instanceN0; 
layout(location = 7) in vec4 instanceN1; 
layout(location = 8) in vec4 instanceN2; 

uniform mat4 transform; 

out vec3 interpolatedNormal; 

out vec4 n0; 
out vec4 n1; 
out vec4 n2; 

void passToFragment() {
    n0 = instanceN0; 
    n1 = instanceN1; 
    n2 = instanceN2; 
}

/*generated end*/

out vec3 worldPos;

void main() {
    passToFragment();
    //worldPos = vertexPosition * instanceTransform.w + instanceTransform.xyz;
    worldPos = (instanceTransform * vec4(vertexPosition, 1.0)).xyz;
	interpolatedNormal = vertexNormal;
	gl_Position = transform * vec4(worldPos, 1.0);
}
