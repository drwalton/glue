#ifndef GLUE_PRTBAKER_HPP_INCLUDED
#define GLUE_PRTBAKER_HPP_INCLUDED

#include <memory>
#include <vector>
#include <opencv2/opencv.hpp>
#include "glue/VectorTypes.hpp"
#include "glue/PrtCoeffts.hpp"

namespace glue
{

//!\brief Class for baking PRT coefficients for a supplied triangle mesh.
//!       Can be used to bake shadowed, or interreflected diffuse PRT.
//!\note The output will be given as color SH coefficients (i.e. each coefficient
//!      in the SH projection is a vec3).
class PrtBaker final {
public:
	PrtBaker(size_t nBands, size_t nSamples);
	~PrtBaker() throw();

	//!\brief Supply the mesh to be used for the baking process. Should be called
	//!       before using any of the bake* functions.
	void setupMesh(
		const std::vector<vec3> &meshVerts,
		const std::vector<vec3> &meshNorms,
		const std::vector<vec2> &meshTexCoords,
		const std::vector<GLuint> &meshIndices,
		size_t width, size_t height);

	//!\brief Bake diffuse shadowed PRT (0 bounces)
	void bakeDiffuseShadowedPrt(PrtCoeffts<vec3>* out, vec3* albedoTex);

	//!\brief Perform an interreflection pass on supplied diffuse shadowed PRT
	//!       coefficients. Should generally not be called manually (use
	//!       bakeDiffuseInterreflectedPrt instead).
	void performInterreflectionPass(PrtCoeffts<vec3>* out, vec3 *albedoTex);

	//!\brief Bake diffuse interreflected PRT, using the specified number of
	//!       light bounces.
	void bakeDiffuseInterreflectedPrt(
		size_t nBounces, PrtCoeffts<vec3>* out, vec3 *albedoTex);

	void setupSpecularMesh(
		const std::vector<vec3> &meshVerts,
		const std::vector<vec3> &meshNorms,
		const std::vector<GLuint> &meshIndices);
	
	//!\brief Bake diffuse shadowed PRT (0 bounces)
	void bakeDiffuseShadowedPrtSpecular(
		PrtCoeffts<vec3>* out, vec3* albedoTex);
	
	//!\brief Perform an interreflection pass on supplied diffuse shadowed PRT
	//!       coefficients. Should generally not be called manually (use
	//!       bakeDiffuseInterreflectedPrt instead).
	void performInterreflectionPassSpecular(
		PrtCoeffts<vec3>* out, vec3 *albedoTex);
	
	//!\brief Bake diffuse interreflected PRT, using the specified number of
	//!       light bounces. Adds a pure specular (mirror reflection) object
	//!       to the lighting simulation.
	//!\note  No coefficients will be baked for the mirror reflection mesh.
	//!\note  Pure specular bounces do not contribute to the bounce count of a
	//!       path. For example, a LSD (light specular diffuse) path can
	//!       contribute even when nBounces is set to 0. Or, an LSDSD path could
	//!       when nBounces is set to 1.
	void bakeDiffuseInterreflectedPrtSpecular(
		size_t nBounces, PrtCoeffts<vec3>* out, vec3 *albedoTex);

private:
	std::vector<glue::vec3> sampleDirs_;
	std::vector<std::vector<float> > sampleShVals_;
	std::unique_ptr<vec4[]> worldPosTex_;
	std::unique_ptr<vec4[]> normalTex_;
	size_t width_, height_, nBands_;

	struct Impl;
	std::unique_ptr<Impl> pimpl_;
};

}

#endif

