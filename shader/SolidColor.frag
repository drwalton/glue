#version 410


layout(std140) uniform cameraBlock
{
	mat4 worldToClip;
	vec4 cameraPos;
	vec4 cameraDir;
};

out vec4 fragColor;

void main()
{
	fragColor = vec4(1,1,1,1);
}
