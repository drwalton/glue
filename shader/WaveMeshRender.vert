
#version 410

layout(location = 0) in vec4 vPos;
layout(location = 1) in vec4 vNorm;

layout(std140) uniform cameraBlock
{
	mat4 worldToClip;
	vec4 cameraPos;
	vec4 cameraDir;
};
uniform mat4 modelToWorld;
uniform mat3 normToWorld;

smooth out vec3 norm;
out float posNeg;

void main()
{
	norm = normToWorld * vNorm.xyz;
	gl_Position = worldToClip *  modelToWorld * vec4(vPos.xyz, 1.0f);
}

