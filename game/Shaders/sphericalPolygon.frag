#version 430 core

uniform vec3 cameraPosition; 

in vec3 interpolatedNormal; 

in vec4 n0; 
in vec4 n1; 
in vec4 n2; 
out vec4 fragColor;

/*generated end*/

vec4 inverseStereographicProjection(vec3 p) {
	float s = p.x * p.x + p.y * p.y + p.z * p.z;
	float a = 2.0f / (s + 1.0f);
	return vec4(
		p.x * a,
		p.y * a,
		p.z * a,
		(s - 1.0f) / (s + 1.0f)
	);
}

in vec3 worldPos;

void main() {
	vec3 normal = normalize(interpolatedNormal);
	float diffuse = dot(-vec3(0, 1, 0), normal);
	diffuse = max(0, diffuse);
	diffuse += 0.5;
	diffuse = clamp(diffuse, 0, 1);
	//fragColor = vec4(interpolatedColor * diffuse, 1.0);

	vec4 pos4 = inverseStereographicProjection(worldPos);
	if (dot(pos4, n0) > 0.0) discard;
	if (dot(pos4, n1) > 0.0) discard;
	if (dot(pos4, n2) > 0.0) discard;
	
	float d = length(worldPos);
	float fogNear = 10.0;
	float fogFar = 60.0;
	d = (fogFar - d) / (fogFar - fogNear);
	//d = smoothstep(110.0, 10.0, d);
	fragColor = vec4(vec3(1.0, 1.0, 0.0) * diffuse * d, 1.0);
	//fragColor = vec4(interpolatedColor.rgb, 1.0);
}
