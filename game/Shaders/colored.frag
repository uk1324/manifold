#version 430 core

in vec3 interpolatedNormal; 

in vec3 color; 
in mat4 model; 
out vec4 fragColor;

/*generated end*/

void main() {
	vec3 normal = normalize(interpolatedNormal);
	float diffuse = dot(-vec3(0, 1, 0), normal);
	diffuse = max(0.0, diffuse);
	diffuse += 0.5;
	diffuse = clamp(diffuse, 0.0, 1.0);
	fragColor = vec4(color * diffuse, 1.0);
	//fragColor = vec4(color, 1.0);
	//fragColor = vec4((normal + 1.0) / 2.0, 1.0);
}
