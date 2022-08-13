#ifndef GLUE_SPHERICALHARMONICS_HPP_INCLUDED
#define GLUE_SPHERICALHARMONICS_HPP_INCLUDED

#include "glue/VectorTypes.hpp"
#include <vector>
#include <string>

namespace glue
{

void dirToSphericalAngles(const vec3 &dir, float *theta, float *phi);
vec3 sphericalAngleToDir(float theta, float phi);

//!\brief Computes the real spherical harmonic SH_l^m(\theta, \phi)
float realSH(int l, int m, float theta, float phi);

//!\brief Given the number of coefficients in an SH projection, find the number of bands.
size_t calcNumSHBands(size_t nCoeffts);

//!\brief Given the number of bands in an SH projection, find the number of coefficients.
size_t calcNumSHCoeffts(size_t nBands);

typedef std::vector<float> SHProjection;

//!\brief A vector containing the coefficients of an SH projection.
typedef std::vector<glue::vec3> SHProjection3;

//!\brief The * operator takes the convolution of the two SH projections.
//!       This is achieved by performing a dot product of the 3 channels.
vec3 convolveSh(const SHProjection3 &lhs, const SHProjection3 &rhs);

const extern SHProjection3 upwardFacingPlanePRT;

void scaleProj(SHProjection3 &proj, float f);

//!\brief Project an arbitrary spherical function to SH.
//!param func The spherical function to project. Should accept two floating-
//!           point arguments, theta and phi, and return a glue::vec3.
template <typename Function>
SHProjection3 projectToSH(int sqrtNSamples, int nBands, Function func);

//!\brief Compute the value of a real spherical harmonic at a given band & angle.
float realSH(int l, int m, float theta, float phi);

//!\brief Compute the value of a real spherical harmonic in a given direction.
//!\note Please ensure the supplied direction is normalized.
float realSH(int l, int m, const vec3 &dir);

//!\brief Evaluate an SH projection at a given location on the sphere
glue::vec3 evaluateSH(SHProjection3 projection, float theta, float phi);

glue::vec3 evaluateSH(SHProjection3 projection, const vec3 &dir);

//!\brief Iterator providing stratified samples over the sphere.
class StratifiedSphericalSampler final
{
public:
	//!\param jitter If true, the samples will be stratified random samples.
	//!       otherwise, the samples will be placed deterministically.
	StratifiedSphericalSampler(size_t sqrtNSamples, bool jitter = true);
	~StratifiedSphericalSampler() throw();
	
	//!\return True if a sample was successfully obtained. False if none remain.
	bool getNextSample(float *theta, float *phi);
private:
	int i_, j_, sqrtNSamples_;
	float sqrWidth_;
	bool jitter_;
};

}

#include "glue/impl/SphericalHarmonics.inl"

#endif

