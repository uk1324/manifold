#version 430 core

layout(location = 0) in vec4 vertexPosition; 
layout(location = 1) in mat4 instanceTransform; 
layout(location = 5) in vec4 instanceN0; 
layout(location = 6) in vec4 instanceN1; 
layout(location = 7) in vec4 instanceN2; 
layout(location = 8) in vec4 instanceN3; 
layout(location = 9) in vec4 instancePlaneNormal; 

uniform mat4 transform; 

out vec4 n0; 
out vec4 n1; 
out vec4 n2; 
out vec4 n3; 
out vec4 planeNormal; 

void passToFragment() {
    n0 = instanceN0; 
    n1 = instanceN1; 
    n2 = instanceN2; 
    n3 = instanceN3; 
    planeNormal = instancePlaneNormal; 
}

/*generated end*/

//out vec3 worldPos;

void main() {
	passToFragment();
	// Interpolation doesn't work correctly with homogenous coordinates.
	vec4 worldPosHomogenous = instanceTransform * vertexPosition;
	//worldPos = worldPosHomogenous.xyz;
	gl_Position = transform * worldPosHomogenous;
}
