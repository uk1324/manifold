#version 430 core

layout(location = 0) in vec3 vertexPosition; 
layout(location = 1) in vec3 vertexNormal; 
layout(location = 2) in vec2 vertexUv; 
layout(location = 3) in mat4 instanceTransform; 
layout(location = 7) in vec4 instanceColor; 

uniform mat4 transform; 

out vec2 uv; 

out vec4 color; 

void passToFragment() {
    color = instanceColor; 
}

/*generated end*/

void main() {
    passToFragment();
	gl_Position = transform * (instanceTransform * vec4(vertexPosition, 1.0));
    uv = vertexUv;
    //gl_Position = transform * (vec4(vertexPosition, 1.0));
}
