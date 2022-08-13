#version 410

layout(location = 0) in vec3 vPos;

uniform mat4 invCamTransform;
uniform mat4 projectMat;

void main()
{
	gl_Position = projectMat * invCamTransform * vec4(vPos, 1.0);
}
