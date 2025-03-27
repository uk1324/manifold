#version 430 core

layout(location = 0) in vec3 vertexPosition; 
layout(location = 1) in vec3 vertexNormal; 
layout(location = 2) in vec4 vertexColor; 

uniform mat4 transform; 
uniform mat4 view; 
uniform mat4 model; 

out vec3 interpolatedNormal; 
out vec4 interpolatedColor; 

/*generated end*/

void main() {
    //m[3][3] = 1;
	gl_Position = transform * (model * vec4(vertexPosition, 1.0));
    interpolatedColor = vertexColor;
    interpolatedNormal = (transpose(inverse(model)) * vec4(vertexNormal, 0.0)).xyz;
}
