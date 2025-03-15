#version 430 core

layout(location = 0) in vec3 vertexPosition; 
layout(location = 1) in vec3 vertexNormal; 
layout(location = 2) in vec3 instanceColor; 
layout(location = 3) in mat4 instanceModel; 

uniform mat4 transform; 
uniform mat4 view; 

out vec3 interpolatedNormal; 

out vec3 color; 
out mat4 model; 

void passToFragment() {
    color = instanceColor; 
    model = instanceModel; 
}

/*generated end*/

void main() {
    passToFragment();
    interpolatedNormal = (transpose(inverse(view * model)) * vec4(vertexNormal, 0.0)).xyz;
    mat4 m = mat4(model);
    //m[3][3] = 1;
	gl_Position = transform * (m * vec4(vertexPosition, 1.0));
}
