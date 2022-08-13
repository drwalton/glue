#include "glue/CubemapLookup.hpp"

#include <array>

namespace glue {

const glue::mat3 glToCubemap = (glue::mat3() <<
	1, 0, 0,
	0, 1, 0,
	0, 0, -1).finished();

const glue::mat3 cubemapToGl = glToCubemap;

int findCubemapFace(vec3 dir)
{
	if (fabsf(dir.x()) >= fabsf(dir.y()) && fabsf(dir.x()) >= fabsf(dir.z())) //x
	{
		return dir.x() >= 0.0f ? 0 : 1;
	}
	else if (fabsf(dir.y()) >= fabsf(dir.z())) //y
	{
		return dir.y() >= 0.0f ? 2 : 3;
	}
	else //z
	{
		return dir.z() >= 0.0f ? 4 : 5;
	}
}

void dirToCubemapPixel(const vec3 &dir, size_t width, size_t height, size_t *face, size_t *row, size_t *col) 
{
	*face = findCubemapFace(dir);
	float s, t;

	// Find texture coordinates.
	switch (*face)
	{
	case 0: //+ve x
		s = -dir.z() / dir.x();
		t = dir.y() / dir.x();
		break;
	case 1: //-ve x
		s = dir.z() / -dir.x();
		t = dir.y() / -dir.x();
		break;
	case 2: //+ve y
		s = dir.x() / dir.y();
		t = -dir.z() / dir.y();
		break;
	case 3: //-ve y
		s = dir.x() / -dir.y();
		t = dir.z() / -dir.y();
		break;
	case 4: //+ve z
		s = dir.x() / dir.z();
		t = dir.y() / dir.z();
		break;
	case 5: //-ve z
		s = -dir.x() / -dir.z();
		t = dir.y() / -dir.z();
		break;
	}

	//Move from [-1,1] range to [0,1] range.
	s = (s + 1.0f) / 2.0f;
	t = (t + 1.0f) / 2.0f;

	//Find integer pixel coords:

	*col = static_cast<int>(s * (width - 1));
	*row = static_cast<int>((1.f - t) * (height - 1));
}

vec3 sampleCubemap(const std::array<cv::Mat, 6>& cubemap, const vec3 & dir)
{
	size_t row, col, face;
	dirToCubemapPixel(dir, cubemap[0].cols, cubemap[0].rows, &face, &row, &col);
	if (cubemap[face].channels() == 3) {
		cv::Vec3b sample = cubemap[face].at<cv::Vec3b>(int(row), int(col));
		return vec3(
			float(sample[0]) / 255.f,
			float(sample[1]) / 255.f,
			float(sample[2]) / 255.f);
	}
	else if (cubemap[face].channels() == 4) {
		cv::Vec4b sample = cubemap[face].at<cv::Vec4b>(int(row), int(col));
		return vec3(
			float(sample[0]) / 255.f,
			float(sample[1]) / 255.f,
			float(sample[2]) / 255.f);
	}
	else {
		throw std::runtime_error("cubemap must have 3 or 4 channels");
	}
}

vec3 sampleCubemapGlCoordSpace(const std::array<cv::Mat, 6>& cubemap, const vec3 & dir)
{
	vec3 cubemapDir = glToCubemap * dir;
	return sampleCubemap(cubemap, cubemapDir);
}

vec3 cubemapPixelToDir(const std::array<cv::Mat, 6>& cubemap, size_t row, size_t col, size_t face)
{
	float s_p = float(col) / float(cubemap[0].cols - 1);
	float t_p = float(row) / float(cubemap[0].rows - 1);
	
	float s = (s_p * 2.0f) - 1.0f;
	float t = (t_p * 2.0f) - 1.0f;

	vec3 dir;
	// Find texture coordinates.
	switch (face)
	{
	case 0: //+ve x
		dir.x() = 1.f;
		dir.z() = -s*dir.x();
		dir.y() = -t*dir.x();
		break;
	case 1: //-ve x
		dir.x() = -1.f;
		dir.z() = -s*dir.x();
		dir.y() = t*dir.x();
		break;
	case 2: //+ve y
		dir.y() = 1.f;
		dir.x() = s * dir.y();
		dir.z() = t*dir.y();
		break;
	case 3: //-ve y
		dir.y() = -1.f;
		dir.x() = -s * dir.y();
		dir.z() = t * dir.y();
		break;
	case 4: //+ve z
		dir.z() = 1.f;
		dir.x() = s * dir.z();
		dir.y() = -t * dir.z();
		break;
	case 5: //-ve z
		dir.z() = -1.f;
		dir.x() = s * dir.z();
		dir.y() = t * dir.z();
		break;
	}

	return dir.normalized();
}

float getCubemapSampleWeight(size_t row, size_t col, size_t height, size_t width)
{
	float u = ((float)(row)/(float)(height))*2.f - 1.f;
	float v = ((float)(col)/(float)(width))*2.f - 1.f;
	float tmp = 1.f + u*u + v*v;
	float weight = 4.f / (sqrtf(tmp)*tmp);
	return weight;
}

}
