#ifndef GLUE_CUBEMAPLOOKUP_HPP_INCLUDED
#define GLUE_CUBEMAPLOOKUP_HPP_INCLUDED

#include "glue/VectorTypes.hpp"
#include <opencv2/opencv.hpp>

//!This file contains various functions to convert between pixel locations in cubemap images
//! and directions in 3D space. Note that here, in all functions, the directions should
//! be expressed in the cubemap's left-handed coordinate system.
//!      +y
//!  -x  +z  +x  -z
//!      -y

namespace glue {

//!\brief Matrix for converting points from the general, right-handed OpenGL 
//!       coordinate space to the left-handed cubemap coordinate space.
extern const glue::mat3 glToCubemap;

//!\brief Matrix for converting points from the left-handed cubemap coordinate 
//!       space to the general, right-handed OpenGL coordinate space.
extern const glue::mat3 cubemapToGl;

int findCubemapFace(vec3 dir);
void dirToCubemapPixel(const vec3 &dir, size_t width, size_t height, size_t *face, size_t *row, size_t *col);
vec3 cubemapPixelToDir(const std::array<cv::Mat, 6>& cubemap, size_t pixelY, size_t pixelX, size_t face);

//!\brief Sample cubemap using a direction in the left-handed cubemap coordinate space.
vec3 sampleCubemap(const std::array<cv::Mat, 6>& cubemap, const vec3 & dir);

//!\brief Sample cubemap using a direction in the general, right-handed OpenGL coordinate space.
vec3 sampleCubemapGlCoordSpace(const std::array<cv::Mat, 6>& cubemap, const vec3 & dir);

//!\brief Get a weight which accounts for the solid angle subtended by a cubemap
//!       texel.
float getCubemapSampleWeight(size_t row, size_t col, size_t height, size_t width);
}


#endif
