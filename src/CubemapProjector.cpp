#include "glue/CubemapProjector.hpp"

#include "glue/CubemapLookup.hpp"
#include <opencv2/opencv.hpp>
#include <array>
#include <thread>
#include <cmath>

namespace glue {

const float AREA_OF_SPHERE = float(4.0 * M_PI);

bool isfinite(cv::Vec3f &v)
{
	return std::isfinite(v[0]) && std::isfinite(v[1]) && std::isfinite(v[2]);
}

CubemapProjector::CubemapProjector(size_t nBands, size_t cubemapWidth)
	:cubemapWidth_(cubemapWidth),
	nBands_(nBands)
{
	projection_.resize(glue::calcNumSHCoeffts(nBands));
	projection_.assign(projection_.size(), glue::vec3::Zero());
	for (size_t i = 0; i < projection_.size(); ++i) {
		sums_.emplace_back(vec3::Zero());
	}
}

CubemapProjector::~CubemapProjector() throw()
{
}

void CubemapProjector::projectCubemap(
	const std::array<cv::Mat, 6>& cubemap,
	const glue::mat3 &rotation)
{
	for (auto &val : sums_) {
		val.set(glue::vec3::Zero());
	}
	for (auto &v : projection_) {
		v = vec3::Zero();
	}
	size_t currNSamples = 0;
	KahanVal<float> weightSum(0.f);
	if (cubemap[0].type() == CV_8UC3 || cubemap[0].type() == CV_8UC4) {

		for (size_t face = 0; face < 6; ++face) {
			for (size_t row = 0; row < cubemap[0].rows; ++row) {
				for (size_t col = 0; col < cubemap[0].cols; ++col) {
					cv::Vec3b sampleRGB8;
					if (cubemap[face].channels() == 3) {
						sampleRGB8 = cubemap[face].at<cv::Vec3b>(row, col);
					} else if (cubemap[face].channels() == 4) {
						cv::Vec4b sampleRGBA8 = cubemap[face].at<cv::Vec4b>(row, col);
						for (size_t i = 0; i < 3; ++i) {
							sampleRGB8[i] = sampleRGBA8[i];
						}
					}
					float weight = getCubemapSampleWeight(
						row, col, cubemap[face].rows, cubemap[face].cols);
					weightSum += weight;
					glue::vec3 dir = cubemapPixelToDir(cubemap, row, col, face);
					dir = cubemapToGl * dir;
					dir.z() = -dir.z();
					dir.x() = -dir.x();
					dir = rotation * dir;
					glue::vec3 sample;
					for (size_t i = 0; i < 3; ++i) {
						sample[i] = float(sampleRGB8[i]) / 255.f;
					}
					sample *= weight;
					for (int l = 0; l < nBands_; ++l) {
						for (int m = -l; m <= l; ++m) {
							float shVal = realSH(l, m, dir);
							sums_[l*(l + 1) + m] += shVal * sample;
						}
					}
					++currNSamples;
				}
			}
		}

		for (size_t i = 0; i < projection_.size(); ++i) {
			projection_[i] = sums_[i].get() * AREA_OF_SPHERE / (weightSum.get());
		}
	} else if (cubemap[0].type() == CV_32FC3 || cubemap[0].type() == CV_32FC4) {
		for (size_t face = 0; face < 6; ++face) {
			for (size_t row = 0; row < cubemap[0].rows; ++row) {
				for (size_t col = 0; col < cubemap[0].cols; ++col) {
					cv::Vec3f sample;
					if (cubemap[face].channels() == 3) {
						sample = cubemap[face].at<cv::Vec3f>(row, col);
					} else if (cubemap[face].channels() == 4) {
						const cv::Vec4f &sampleRGBA = cubemap[face].at<cv::Vec4f>(row, col);
						for (size_t i = 0; i < 3; ++i) {
							sample[i] = sampleRGBA[i];
						}
					}
					if (isfinite(sample)) { //NaN check
						float weight = getCubemapSampleWeight(row, col, 
							cubemap[face].rows, cubemap[face].cols);
						weightSum += weight;
						glue::vec3 dir = cubemapPixelToDir(cubemap, row, col, face);
						dir = cubemapToGl * dir;
						dir.z() = -dir.z();
						dir.x() = -dir.x();
						dir = rotation * dir;
						sample *= weight;
						for (int l = 0; l < nBands_; ++l) {
							for (int m = -l; m <= l; ++m) {
								float shVal = realSH(l, m, dir);
								sums_[l*(l + 1) + m] += 
									shVal * glue::vec3(sample[0], sample[1], sample[2]);
								if (shVal != shVal) {
									std::cout << "Invalid at face:" << face <<
										" row:" << row << " col:" << col << std::endl;
									std::cout << sample << std::endl;
									std::cin.get();
								}
							}
						}
					}
				}
			}
		}


		for (size_t i = 0; i < projection_.size(); ++i) {
			if (weightSum.get() <= 1e-6f) {
				projection_[i] = vec3::Ones();
			} else {
				projection_[i] = sums_[i].get() * AREA_OF_SPHERE / (weightSum.get());
				if (projection_[i] != projection_[i]) {
					projection_[i] = vec3::Zero();
				}
			}
		}

	} else {
		throw std::runtime_error("invalid matrix type input to CubemapProjector");
	}
}

void CubemapProjector::projectCubemap(size_t faceWidth, size_t nChannels, float * posx, float * negx, float * posy, float * negy, float * posz, float * negz)
{
	std::array<cv::Mat, 6> cubemap;
	int type;
	if (nChannels == 3) {
		type = CV_32FC3;
	} else if (nChannels == 4) {
		type = CV_32FC4;
	} else {
		throw std::runtime_error("Invalid channel count.");
	}
	cubemap[0] = cv::Mat(cv::Size(faceWidth, faceWidth), type, posx);
	cubemap[1] = cv::Mat(cv::Size(faceWidth, faceWidth), type, negx);
	cubemap[2] = cv::Mat(cv::Size(faceWidth, faceWidth), type, posy);
	cubemap[3] = cv::Mat(cv::Size(faceWidth, faceWidth), type, negy);
	cubemap[4] = cv::Mat(cv::Size(faceWidth, faceWidth), type, posz);
	cubemap[5] = cv::Mat(cv::Size(faceWidth, faceWidth), type, negz);
	projectCubemap(cubemap);
}

const SHProjection3 & CubemapProjector::projection() const
{
	return projection_;
}

SHProjection3 & CubemapProjector::proj()
{
	return projection_;
}

}
