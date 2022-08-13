#ifndef GLUE_CUBEMAPPROJECTOR_HPP_INCLUDED
#define GLUE_CUBEMAPPROJECTOR_HPP_INCLUDED

#define _USE_MATH_DEFINES
#include <vector>
#include <opencv2/opencv.hpp>
#include "glue/VectorTypes.hpp"
#include "glue/PrtCoeffts.hpp"
#include "glue/KahanVal.hpp"

namespace glue
{

class CubemapProjector final {
public:
	explicit CubemapProjector(size_t nBands, size_t cubemapWidth);
	~CubemapProjector() throw();

	//!\brief Project cubemap to SH, sampling at every cubemap pixel.
	//!\note Intended for use on small mipmap levels of cubemaps, will be very slow
	//!      on a full 512x512 cubemap.
	void projectCubemap(const std::array<cv::Mat, 6> &cubemap, 
		const glue::mat3 &rotation = glue::mat3::Identity());

	void projectCubemap(
		size_t faceWidth,
		size_t nChannels,
		float *posx, float *negx, 
		float *posy, float *negy, 
		float *posz, float *negz);

	const SHProjection3 &projection() const;
	SHProjection3 &proj();
private:
	SHProjection3 projection_;
	std::vector<KahanVal<vec3>> sums_;
	const size_t cubemapWidth_;
	const size_t nBands_;
};
	
}

#endif

