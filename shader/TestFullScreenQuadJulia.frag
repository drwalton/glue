#version 410

smooth in vec2 texCoord;

uniform dvec2 offset;
uniform double scale;
uniform int maxIters;
uniform double aspect;
uniform dvec2 c;
uniform int power;

const double maxDist = 2.0;

out vec4 color;

dvec2 func(dvec2 z, dvec2 c)
{
	for(int i = 1; i < power; ++i) {
		z = dvec2(z.x*z.x - z.y*z.y,
				  2.f*z.x*z.y);
	}
	return z + c;
}

vec4 nItersToColor(int iters)
{
	float v = float(iters) / float(maxIters);
	return vec4(0, v, v, 1);
}

void main()
{
	dvec2 screenPos = texCoord - dvec2(0.5, 0.5);
	screenPos.x *= aspect;
	dvec2 dscreenPos = screenPos * scale;
	dvec2 z = dscreenPos + offset;
	
	for(int i = 0; i < maxIters; ++i) {
		z = func(z, c);
		if(length(z) > maxDist) {
			color = nItersToColor(i);
			return;
		}
	}
	
	color = vec4(0, 0, 0, 1);
}

