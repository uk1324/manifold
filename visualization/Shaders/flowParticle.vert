#version 430 core

layout(location = 0) in vec3 vertexPosition; 
layout(location = 1) in vec3 vertexNormal; 
layout(location = 2) in vec2 vertexUv; 
layout(location = 3) in vec4 instancePositionScale; 
layout(location = 4) in vec4 instanceColor; 

uniform mat4 transform; 
uniform mat4 rotate; 

out vec2 uv; 

out vec4 color; 

void passToFragment() {
    color = instanceColor; 
}

/*generated end*/

void main() {
    passToFragment();
    vec3 pos3 = vertexPosition;
    pos3 -= vec3(0.5, 0.5, 0.0);
    float scale = instancePositionScale.w;
    pos3 *= scale;
    pos3 = mat3(rotate) * pos3;
    vec3 translation = instancePositionScale.xyz;
    pos3 += translation;
	gl_Position = transform * vec4(pos3, 1.0);
    uv = vertexUv;
    //gl_Position = transform * (vec4(vertexPosition, 1.0));
}
