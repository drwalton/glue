#version 410
//Billboarding shader designed to turn points into hexagons.


layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in VertexData {
	vec3 norm;
	float radius;
} VertexOut[];

//TODO change to camera block.
uniform mat4 worldToClip;
uniform vec3 cameraPos;
uniform vec3 cameraDir;

smooth out float depth;

void main()
{
	vec3 center = gl_in[0].gl_Position.xyz;
	vec3 across = normalize(cross(VertexOut[0].norm, vec3(0.0, 1.0, 0.0)));
	vec3 up     = -normalize(cross(VertexOut[0].norm, across));

	vec3 vert;
	vert = center + VertexOut[0].radius * (
		-1.0 * across +
		-1.0 * up);
	depth = dot(cameraDir, vert - cameraPos);
	gl_Position = worldToClip * vec4(vert, 1.0);
	EmitVertex();
	vert = center + VertexOut[0].radius * (
		-1.0 * across +
		1.0 * up);
	depth = dot(cameraDir, vert - cameraPos);
	gl_Position = worldToClip * vec4(vert, 1.0);
	EmitVertex();
	vert = center + VertexOut[0].radius * (
		1.0 * across +
		-1.0 * up);
	depth = dot(cameraDir, vert - cameraPos);
	gl_Position = worldToClip * vec4(vert, 1.0);
	EmitVertex();
	vert = center + VertexOut[0].radius * (
		1.0 * across +
		1.0 * up);
	depth = dot(cameraDir, vert - cameraPos);
	gl_Position = worldToClip * vec4(vert, 1.0);
	EmitVertex();

	EndPrimitive();
}

