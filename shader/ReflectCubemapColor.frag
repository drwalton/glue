#version 410

in vec3 norm;
in vec3 worldSpacePos;

uniform samplerCube cubemap;
uniform float lodLevel;
uniform vec3 objColor;

layout(std140) uniform cameraBlock
{
	mat4 worldToClip;
	vec4 cameraPos;
	vec4 cameraDir;
};

out vec4 color;

void main()
{
	vec3 lookupDir = reflect(
		normalize(pos - vec3(camPosModelSpace)), normalize(norm));
	lookupDir.x = -lookupDir.x;
	color = texture(cubemap, lookupDir);
	color = vec4(objColor * color.xyz, 1.0);
}

