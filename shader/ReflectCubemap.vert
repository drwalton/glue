#version 410

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNorm;

layout(std140) uniform cameraBlock
{
	mat4 worldToClip;
	vec4 cameraPos;
	vec4 cameraDir;
};

uniform mat4 modelToWorld;
uniform mat3 normToWorld;

smooth out vec3 norm;
smooth out vec3 pos;

void main()
{
	vec4 worldPos4 = modelToWorld * vec4(vPos, 1.0f);
	gl_Position = worldToClip * worldPos4;
	norm = normToWorld * vNorm;
	pos = worldPos4.xyz / worldPos4.w;
}
