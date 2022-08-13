#version 410

layout(location=0) in vec3 vert;
layout(location=1) in vec3 norm;
layout(location=4) in float radius;

out VertexData {
	vec3 norm;
	float radius;
} VertexOut;

void main()
{
	gl_Position = vec4(vert, 1.0);
	VertexOut.norm = norm;
	VertexOut.radius = radius;
}
