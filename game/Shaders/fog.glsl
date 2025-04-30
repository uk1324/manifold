float fogScale(vec3 pos) {
	float d = length(pos);
	float fogNear = 10.0;
	float fogFar = 20.0;
	d = (fogFar - d) / (fogFar - fogNear);
	return d;
}