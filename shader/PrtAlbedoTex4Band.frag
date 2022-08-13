#version 410

//Encourage driver to unroll all loops (nVidia).
#pragma optionNV (unroll all)

smooth in vec2 texCoord;

uniform sampler2DArray prtCoeffts;
uniform sampler2D albedoTex;
uniform vec3[16] lightingCoeffts;

out vec4 color;

void main()
{
	vec3 color3 = vec3(0,0,0);
	vec4 coefft4;
	float coefft;

	for(int i = 0; i < 16; ++i) {
		coefft4 = texture(prtCoeffts, vec3(texCoord, i));
		color3 += vec3(coefft4.x, coefft4.x, coefft4.x) * lightingCoeffts[i];
	}

	vec4 albedo = texture(albedoTex, texCoord);

	color3 *= albedo.xyz;

	color = vec4(color3, 1);
	//color = texture(prtCoeffts, vec3(texCoord, 0));
}

