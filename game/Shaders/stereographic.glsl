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