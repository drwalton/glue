#version 410

smooth in vec3 norm;

layout(std140) uniform cameraBlock
{
	mat4 worldToClip;
	vec4 cameraPos;
	vec4 cameraDir;
};

out vec4 color;

void main()
{
	float light = 0.3 + max(dot(norm, cameraDir.xyz), 0.0);

	color = vec4(0.0, 0.0, 1.0, 0.0) * light;
}
