#version 410

in vec3 norm;
in vec3 pos;

uniform samplerCube cubemap;
uniform float eta;

uniform vec3 camPosModelSpace;

out vec4 color;

void main()
{
	vec3 lookupDir = refract(
		normalize(pos - vec3(camPosModelSpace)), normalize(norm), eta);
	lookupDir.x = -lookupDir.x;
	color = texture(cubemap, lookupDir);
	color.w = 1;
	//color = vec4(1, 0, 0, 1);
}

