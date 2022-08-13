#version 410

smooth in vec3 norm;

layout(std140) uniform cameraBlock
{
	mat4 worldToClip;
	vec4 cameraPos;
	vec4 cameraDir;
};

out vec4 fragColor;

uniform vec4 warmColor = vec4(0.0, 1.0, 0.0, 1.0);
uniform vec4 coolColor = vec4(0.0, 0.2, 0.0, 1.0);

void main()
{
	vec3 nNorm = normalize(norm);
	float vDotN = max(dot(-cameraDir.xyz, nNorm), 0.0);
	fragColor = mix(coolColor, warmColor, vDotN);
}
