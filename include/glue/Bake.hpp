#ifndef GLUE_BAKE_HPP_INCLUDED
#define GLUE_BAKE_HPP_INCLUDED

#include <vector>
#include <opencv2/opencv.hpp>
#include "glue/VectorTypes.hpp"
#include "glue/PrtCoeffts.hpp"

namespace glue
{

void renderWorldPosAndNormals(
	const std::vector<vec3> &vertices,
	const std::vector<vec3> &normals,
	const std::vector<vec2> &texCoords,
	const std::vector<GLuint> &indices,
	vec4 *worldPosImage, 
	vec4 *normalImage, 
	size_t width, size_t height);

void bakeAmbientOcclusionTexture(
	const std::vector<vec3> &vertices,
	const std::vector<vec3> &normals,
	const std::vector<vec2> &texCoords,
	const std::vector<GLuint> &indices,
	float *outputTexture,
	vec4 *worldPosImage, 
	vec4 *normalImage, 
	size_t width, size_t height,
	size_t nSamples = 100);

PrtCoeffts<float> bakeDiffuseShadowedPrtCoeffts(
	const std::vector<vec3>& vertices,
	const std::vector<vec3>& normals,
	const std::vector<vec2>& texCoords,
	const std::vector<GLuint>& indices,
	vec4 * worldPosImage, vec4 * normalImage,
	size_t width, size_t height,
	size_t nBands, size_t nSamples = 100);

PrtCoeffts<float> bakeDiffuseInterreflectedPrtCoeffts(
	const std::vector<vec3>& vertices,
	const std::vector<vec3>& normals,
	const std::vector<vec2>& texCoords,
	const std::vector<GLuint>& indices,
	vec4 * worldPosImage, vec4 * normalImage,
	const cv::Mat &albedoTexture,
	size_t width, size_t height,
	size_t nBands, size_t nSamples = 100);

//!\brief Project cubemap to SH, taking nSamples random samples.
SHProjection3 projectCubemap(
	const std::array<cv::Mat, 6> &cubemap,
	size_t nSamples, size_t nBands);
	
//!\brief Project cubemap to SH, sampling at every cubemap pixel.
//!\note Intended for use on small mipmap levels of cubemaps, will be very slow
//!      on a full 512x512 cubemap.
SHProjection3 projectCubemap(
	const std::array<cv::Mat, 6> &cubemap,
	size_t nBands);

void shToCubemap(const SHProjection3 &proj,
	std::array<cv::Mat, 6> *cubemap);

void viewPrtCoefftsUnderLighting(
	const PrtCoeffts<float> &prtCoeffts,
	const SHProjection3 &lighting);

}

#endif

