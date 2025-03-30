#version 430 core

layout(location = 0) in vec3 vertexPosition; 
layout(location = 1) in vec3 vertexNormal; 
layout(location = 2) in vec4 vertexColor; 
layout(location = 3) in mat4 instanceModel; 

uniform mat4 transform; 
uniform mat4 view; 

out vec3 interpolatedNormal; 
out vec4 interpolatedColor; 

/*generated end*/

void main() {
    //m[3][3] = 1;
	gl_Position = transform * (instanceModel * vec4(vertexPosition, 1.0));
    interpolatedColor = vertexColor;
    interpolatedNormal = (transpose(inverse(instanceModel)) * vec4(vertexNormal, 0.0)).xyz;
}
