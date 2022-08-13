#version 410

layout(location = 0) in vec3 vPos;
layout(location = 3) in vec3 vColor;

layout(std140) uniform cameraBlock
{
	mat4 worldToClip;
	vec4 cameraPos;
	vec4 cameraDir;
};
uniform mat4 modelToWorld;
uniform mat3 normToWorld;

smooth out vec3 color;

void main()
{
	color = vColor;
	gl_Position = worldToClip *  modelToWorld * vec4(vPos.xyz, 1.0f);
}
