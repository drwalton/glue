namespace glue
{

/* Used for testing if floats are 0 */
const float EPS = 10e-6f;

template <typename Function>
SHProjection3 projectToSH(int sqrtNSamples, int nBands, Function func)
{
	SHProjection3 projection;
	/* Initialise vector of coeffts with zeros */
	for(int l = 0; l < nBands; ++l) {
		for(int m = -l; m <= l; ++m) {
			projection.push_back(glue::vec3(0.0f));
		}
	}
	
	/* Perform stratified random sampling over the sphere */
	StratifiedSphericalSampler sampler(sqrtNSamples, true);
	float theta, phi;
	
	while(sampler.getNextSample(&theta, &phi)) {
		for(int l = 0; l < nBands; ++l) {
    		for(int m = -l; m <= l; ++m) {
    			glue::vec3 val = func(theta, phi);
    			/* Do not calculate SH if unnecessary (val is 0) */
    			if(fabs(val.x()) < EPS &&
    			fabs(val.y()) < EPS &&
    			fabs(val.z()) < EPS) continue;
    			projection[l*(l+1) + m] +=
					val * glue::vec3(realSH(l, m, theta, phi));
    		}
		}
	}

	
	/* Normalize coefficients */
	int nSamples = sqrtNSamples * sqrtNSamples;
	for(glue::vec3 &i : projection) {
		i *= 4.0 * M_PI / static_cast<float>(nSamples);
	}
	
	return projection;
}
	
}
