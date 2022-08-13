#version 410

smooth in vec3 omniDir;

uniform sampler2D omniIm;
uniform sampler2D omniImMask;

out vec4 color;

#pragma include camera/elpCamera.glsl

void main()
{
	vec3 omniDirN = normalize(omniDir);
	omniDirN.z = - omniDirN.z;

	//Check dot product (i.e. if point is directly behind camera).
	float dotProd = dot(omniDirN, vec3(0.f, 0.f, 1.f));

	if(dotProd < minDotProduct) {
		discard;
	}

	//Move point into image space.
	float den = omniDirN.z + length(omniDirN) * e;
	vec2 imPos = vec2(
        fx * (omniDirN.x / den) + cx,
        fy * (omniDirN.y / den) + cy);

	if(imPos.x < 0) {
		discard;
	}

	vec2 samplePos = vec2(imPos.x/omniWidth, 1.0 - imPos.y/omniWidth);
	if(texture(omniImMask, samplePos).r == 0) {
		discard;
	}

	color = texture(omniIm, samplePos);
}
