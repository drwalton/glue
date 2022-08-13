#version 410

smooth in vec3 norm;
in float posNeg;

layout(std140) uniform cameraBlock
{
	mat4 worldToClip;
	vec4 cameraPos;
	vec4 cameraDir;
};

out vec4 fragColor;

const vec4 negWarmColor = vec4(0.0, 0.0, 1.0, 1.0);
const vec4 negCoolColor = vec4(0.0, 0.0, 0.2, 1.0);
const vec4 posWarmColor = vec4(0.0, 1.0, 0.0, 1.0);
const vec4 posCoolColor = vec4(0.0, 0.2, 0.0, 1.0);

void main()
{
	vec3 nNorm = normalize(norm);
	float vDotN = max(dot(cameraDir.xyz, nNorm), 0.0);
	if(posNeg >= 0.)
	{
		fragColor = mix(posCoolColor, posWarmColor, vDotN);
	}
	else
	{
		fragColor = mix(negCoolColor, negWarmColor, vDotN);
	}
	//fragColor = vec4(1., 0., 0., 1.);
}
