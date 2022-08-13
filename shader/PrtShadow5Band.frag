#version 410

//Encourage driver to unroll all loops (nVidia).
#pragma optionNV (unroll all)

smooth in vec2 texCoord;

uniform sampler2DArray prtCoeffts;
uniform vec3[25] lightingCoeffts;
uniform vec3 realSurfaceAlbedo;

out vec4 color;

const float SQRT_3 = 1.732050807568877293527;

void main()
{
	vec3 color3 = vec3(0,0,0);
	vec4 coefft4;
	float coefft;

	for(int i = 0; i < 25; ++i) {
		coefft4 = texture(prtCoeffts, vec3(texCoord, i));
		color3 += vec3(coefft4.x, coefft4.x, coefft4.x) * lightingCoeffts[i];
	}

	//* OLD LDR VERSION *//
	//color3 *= shadowPower;
	//float mean = (color3.x + color3.y + color3.z) / 3.f;
	//color3 = vec3(mean, mean, mean);
	//float alpha = length(color3) / SQRT_3;
	//color = vec4(vec3(1,1,1) + color3, alpha);

	//* NEW HDR VERSION *//
	color = vec4(realSurfaceAlbedo * color3, -1.0);

	//color = vec4(1,1,1,1);
	//color = texture(prtCoeffts, vec3(texCoord, 0));
}

