#version 410

layout(location = 1) in vec3 vNorm;
layout(location = 2) in vec2 vTex;

smooth out vec3 normal;

void main()
{
	normal = vNorm;
	gl_Position = vec4(vTex*2 - vec2(1,1), 0, 1);
}

