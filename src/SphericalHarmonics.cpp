#define _USE_MATH_DEFINES
#include "glue/SphericalHarmonics.hpp"
#include <exception>

namespace glue
{

const SHProjection3 upwardFacingPlanePRT = {
	vec3(0.0223746, 0.0223746, 0.0223746),
	vec3(-0.0255797,-0.0255797,-0.0255797),
	vec3(-0.000191027,-0.000191027,-0.000191027),
	vec3(0.000165369,0.000165369,0.000165369),
	vec3(-0.00017531,-0.00017531,-0.00017531),
	vec3(0.000184577,0.000184577,0.000184577),
	vec3(-0.00600632,-0.00600632,-0.00600632),
	vec3(-0.000110629,-0.000110629,-0.000110629),
	vec3(-0.0102116,-0.0102116,-0.0102116),
	vec3(-0.000744635,-0.000744635,-0.000744635),
	vec3( 7.67762e-05,7.67762e-05,7.67762e-05),
	vec3( -0.00036499,- 0.00036499,- 0.00036499),
	vec3( -5.30683e-05,- 5.30683e-05,- 5.30683e-05),
	vec3( -8.80663e-05,- 8.80663e-05,- 8.80663e-05),
	vec3(0.000101528,0.000101528,0.000101528),
	vec3( -2.34127e-05,- 2.34127e-05,- 2.34127e-05)
};

const float shZero = 0.5 * sqrt(1.0 / M_PI);

void dirToSphericalAngles(const vec3 &dir, float *theta, float *phi)
{
	*theta = acosf(dir.z());
	*phi = 0.f;
	if (fabsf(dir.x()) > 0.f || fabsf(dir.y()) > 0.f) {
		*phi = atan2f(dir.y(), dir.x());
		if (*phi < 0.f) {
			*phi = static_cast<float>(2.f*M_PI + *phi);
		}
	}
	if(*theta != *theta || *phi != *phi) {
		std::stringstream ss;
		ss << "Unable to convert direction " << dir << " to spherical angles: "
			<< "result: theta " << theta << " phi " << phi;
		throw std::runtime_error(ss.str());
	}
}

vec3 sphericalAngleToDir(float theta, float phi)
{
	return vec3(
		cos(phi) * sin(theta),
		sin(phi) * sin(theta),
		cos(theta));
}

int fact(int i)
{
	if(i == 0) return 1;
	int ans = 1;
	while(i > 0)
	{
		ans *= i;
		--i;
	}
	return ans;
}

int dblFact(int i)
{
	if(i == 0) return 1;
	int ans = 1;
	while(i > 0)
	{
		ans *= i;
		i -= 2;
	}
	return ans;
}

float K(int l, int m)
{
	return sqrt(
		(((float) (2*l + 1)) / (4.0f * M_PI)) *
		((float) fact(l - abs(m)) / (float) fact(l + abs(m)))
		);
}

float P(int l, int m, float x)
{
	if(l < m)
		throw(new std::runtime_error(
			"l,m out of range in call to P(). Require l < m."));
	
	if(l == m)
		return (m % 2 ? -1.0f : 1.0f) *
		(float) dblFact(2*m - 1) *
		pow((float) 1.0f - x*x, (float) m / 2);
	if(l == m+1)
		return x *
		((float) (2*m + 1)) *
		P(m, m, x);
	//else
	return (1 / ((float) (l - m))) *
	((x *
	  ((float) (2*l - 1)) *
	  P(l-1, m, x))
	 -
	 (((float) l + m - 1) *
	  P(l-2, m, x)));
}

float realSH(int l, int m, float theta, float phi)
{
	if(l < 0 || l < m || -l > m)
		throw(new std::runtime_error(
			"l,m out of range in call to realSH(). Require -l <= m <= l."));
	if (l == 0) {
		//Trivial case: this band is constant.
		return shZero;
	}
	if (m > 0) {
		return static_cast<float>(M_SQRT2 * K(l, m) * cos(m * phi) * P(l, m, cos(theta)));
	}
	if (m < 0) {
		return static_cast<float>(M_SQRT2 * K(l, m) * sin(-m * phi) * P(l, -m, cos(theta)));
	}
	// m == 0
	return K(l, 0) * P(l, 0, cos(theta));
}

size_t calcNumSHBands(size_t nCoeffts)
{
	size_t nBands = 1;
	while (nBands*nBands < nCoeffts) {
		++nBands;
	}
	if (nBands*nBands == nCoeffts) {
		return nBands;
	}
	throw std::runtime_error("Invalid number of SH coefficients passed to calcNumSHBands");
}

size_t calcNumSHCoeffts(size_t nBands)
{
	return nBands*nBands;
}

vec3 convolveSh(const SHProjection3 & lhs, const SHProjection3 & rhs)
{
	vec3 soln = vec3::Zero();
	for (size_t i = 0; i < lhs.size(); ++i) {
		soln += (lhs[i].array() * rhs[i].array()).matrix();
	}
	return soln;
}

void scaleProj(SHProjection3 & proj, float f)
{
	for (size_t i = 0; i < proj.size(); ++i) {
		proj[i] *= f;
	}
}

float realSH(int l, int m, const vec3 & dir)
{
	//The fast versions here are from 
	// https://en.wikipedia.org/wiki/Table_of_spherical_harmonics
	// This uses a different coordinate space, so we negate x and y
	// to make the results consistent.
	float
		x = -dir.x(),
		y = -dir.y(),
		z = dir.z();
	if (l == 0 && m == 0) {
		return shZero;
	} else if (l == 1) {
		const float K = sqrt(.75 /  M_PI);
		if (m == -1) {
			return K * y;
		} else if (m == 0) {
			return K * z;
		} else if (m == 1) {
			return K * x;
		}
	} else if (l == 2) {
		const float K = 0.5 * sqrt(15.0 / M_PI);
		if (m == -2) {
			return K * x * y;
		} else if (m == -1) {
			return K * y * z;
		} else if (m == 0) {
			const float K0 = 0.25 * sqrt(5.0 / M_PI);
			return K0 * (-x*x - y*y + 2.0*z*z);
		} else if (m == 1) {
			return K * z * x;
		} else if (m == 2) {
			const float K2 = 0.25 * sqrt(15.0 / M_PI);
			return K2 * (x * x - y * y);
		} 
	} else if (l == 3) {
		if (m == -3) {
			const float K = 0.25 * sqrt(17.25 / M_PI);
			return K * (3.0*x*x - y*y) * y;
		} else if (m == -2) {
			const float K = 0.5 * sqrt(105.0 / M_PI);
			return K * x*y*z;
		} else if (m == -1) {
			const float K = 0.25 * sqrt(10.5 / M_PI);
			return K * y * (4.0 * z * z - x*x - y*y);
		} else if (m == 0) {
			const float K = 0.25 * sqrt(7.0 / M_PI);
			return K * z * (2.0*z*z - 3.0*x*x - 3.0*y*y);
		} else if (m == 1) {
			const float K = 0.25 * sqrt(10.5 / M_PI);
			return K * x * (4.0 * z * z - x*x - y*y);
		} else if (m == 2) {
			const float K = 0.25 * sqrt(105.0 / M_PI);
			return K * (x*x - y*y) * z;
		} else if (m == 3) {
			const float K = 0.25 * sqrt(17.25 / M_PI);
			return K * (x*x - 3.0*y*y) * x;
		}
	} else if (l == 4) {
		if (m == -4) {
			const float K = 0.75 * sqrt(35.0 / M_PI);
			return K * x*y * (x*x - y*y);
		} else if (m == -3) {
			const float K = 0.75 * sqrt(17.5 / M_PI);
			return K * (3.0*x*x - y*y) * y*z;
		} else if (m == -2) {
			const float K = 0.75 * sqrt(5.0 / M_PI);
			return K * x*y * (7.0*z*z - 1.0);
		} else if (m == -1) {
			const float K = 0.75 * sqrt(2.5 / M_PI);
			return K * y*z * (7.0*z*z - 3.0);
		} else if (m == -0) {
			const float K = (3.0 / 16.0) * sqrt(1.0 / M_PI);
			return K * (35.0*z*z*z*z - 30.0*z*z + 3.0);
		} else if (m == 1) {
			const float K = 0.75 * sqrt(2.5 / M_PI);
			return K * x*z * (7.0*z*z - 3.0);
		} else if (m == 2) {
			const float K = (3.0 / 8.0) * sqrt(5.0 / M_PI);
			return K * (x*x - y*y) * (7.0*z*z - 1.0);
		} else if (m == 3) {
			const float K = 0.75 * sqrt(17.5 / M_PI);
			return K * (x*x - 3.0*y*y) * x*z;
		} else if (m == 4) {
			const float K = (3.0 / 16.0) * sqrt(35.0 / M_PI);
			return K * (x*x * (x*x - 3.0*y*y) - y*y * (3.0*x*x - y*y));
		}
	}
	float theta, phi;
	dirToSphericalAngles(dir, &theta, &phi);
	return realSH(l, m, theta, phi);
}

float randf(float low, float high)
{
	float r = (float) rand() / (float) RAND_MAX;
	return low + ((high - low) * r);
}


glue::vec3 evaluateSH(SHProjection3 projection, float theta, float phi)
{
	glue::vec3 value = glue::vec3::Zero();
	
	for(unsigned i = 0; i < projection.size(); ++i)
	{
		int l = static_cast<int>(sqrt(static_cast<float>(i)));
		int m = i - l*(l+1);
		value += projection[i] * (float) realSH(l, m, theta, phi);
	}
	
	return value;
}

glue::vec3 evaluateSH(SHProjection3 projection, const vec3 & dir)
{
	glue::vec3 value = glue::vec3::Zero();

	for (unsigned i = 0; i < projection.size(); ++i) {
		int l = static_cast<int>(sqrt(static_cast<float>(i)));
		int m = i - l*(l + 1);
		value += projection[i] * (float)realSH(l, m, dir);
	}

	return value;
}
	
StratifiedSphericalSampler::StratifiedSphericalSampler(
	size_t sqrtNSamples, bool jitter)
	:i_(0), j_(0), sqrtNSamples_(int(sqrtNSamples)),
	sqrWidth_(1.f / (float) sqrtNSamples),
	jitter_(jitter)
{}
StratifiedSphericalSampler::~StratifiedSphericalSampler() throw()
{}
bool StratifiedSphericalSampler::getNextSample(float *theta, float *phi)
{
	++i_;
	if(i_ >= sqrtNSamples_) {
		i_ = 0;
		++j_;
	}
	if(j_ >= sqrtNSamples_) {
		return false;
	}

	float u = (i_ * sqrWidth_);
	float v = (j_ * sqrWidth_);
	
	//Jitter samples from central point.
	if(jitter_) {
    	u += randf(0, sqrWidth_);
    	v += randf(0, sqrWidth_);
	}
	
	*theta = acos((2 * u) - 1);
	*phi = static_cast<float>(2 * M_PI * v);
	
	return true;
}

}
