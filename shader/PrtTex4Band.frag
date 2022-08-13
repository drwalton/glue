#version 410

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

	coefft4 = texture(prtCoeffts, vec3(texCoord, 0));
	color3 += vec3(coefft4.x, coefft4.x, coefft4.x) * lightingCoeffts[0];
	coefft4 = texture(prtCoeffts, vec3(texCoord, 1));
	color3 += vec3(coefft4.x, coefft4.x, coefft4.x) * lightingCoeffts[1];
	coefft4 = texture(prtCoeffts, vec3(texCoord, 2));
	color3 += vec3(coefft4.x, coefft4.x, coefft4.x) * lightingCoeffts[2];
	coefft4 = texture(prtCoeffts, vec3(texCoord, 3));
	color3 += vec3(coefft4.x, coefft4.x, coefft4.x) * lightingCoeffts[3];

	coefft4 = texture(prtCoeffts, vec3(texCoord, 4));
	color3 += vec3(coefft4.x, coefft4.x, coefft4.x) * lightingCoeffts[4];
	coefft4 = texture(prtCoeffts, vec3(texCoord, 5));
	color3 += vec3(coefft4.x, coefft4.x, coefft4.x) * lightingCoeffts[5];
	coefft4 = texture(prtCoeffts, vec3(texCoord, 6));
	color3 += vec3(coefft4.x, coefft4.x, coefft4.x) * lightingCoeffts[6];
	coefft4 = texture(prtCoeffts, vec3(texCoord, 7));
	color3 += vec3(coefft4.x, coefft4.x, coefft4.x) * lightingCoeffts[7];

	coefft4 = texture(prtCoeffts, vec3(texCoord, 8));
	color3 += vec3(coefft4.x, coefft4.x, coefft4.x) * lightingCoeffts[8];
	coefft4 = texture(prtCoeffts, vec3(texCoord, 9));
	color3 += vec3(coefft4.x, coefft4.x, coefft4.x) * lightingCoeffts[9];
	coefft4 = texture(prtCoeffts, vec3(texCoord, 10));
	color3 += vec3(coefft4.x, coefft4.x, coefft4.x) * lightingCoeffts[10];
	coefft4 = texture(prtCoeffts, vec3(texCoord, 11));
	color3 += vec3(coefft4.x, coefft4.x, coefft4.x) * lightingCoeffts[11];

	coefft4 = texture(prtCoeffts, vec3(texCoord, 12));
	color3 += vec3(coefft4.x, coefft4.x, coefft4.x) * lightingCoeffts[12];
	coefft4 = texture(prtCoeffts, vec3(texCoord, 13));
	color3 += vec3(coefft4.x, coefft4.x, coefft4.x) * lightingCoeffts[13];
	coefft4 = texture(prtCoeffts, vec3(texCoord, 14));
	color3 += vec3(coefft4.x, coefft4.x, coefft4.x) * lightingCoeffts[14];
	coefft4 = texture(prtCoeffts, vec3(texCoord, 15));
	color3 += vec3(coefft4.x, coefft4.x, coefft4.x) * lightingCoeffts[15];

	color3 *= texture(albedoTex, texCoord);

	color = vec4(color3, 1);
	//color = texture(prtCoeffts, vec3(texCoord, 0));
}

