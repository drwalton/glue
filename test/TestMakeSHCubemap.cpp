#include "glue/Directories.hpp"
#include "glue/Bake.hpp"
#include "glue/PrtCoeffts.hpp"
#include "glue/SphericalHarmonics.hpp"
#include <FL/Fl_Native_File_Chooser.H>
#include <opencv2/opencv.hpp>
#include <array>
#include <SDL.h>

const std::array<std::string, 6> cubemapFilenames = {
	"negx.jpg", "negy.jpg", "negz.jpg",
	"posx.jpg", "posy.jpg", "posz.jpg"
};

int main(int argc, char *argv[])
{
	glue::SHProjection3 proj;
	proj.resize(glue::calcNumSHCoeffts(4));
	for (size_t i = 1; i < proj.size(); ++i) {
		proj[i] = glue::vec3::Zero();
	}
	proj[0] = glue::vec3(1.f, 0.f, 0.f);
	proj[1] = glue::vec3(0.f, -5.f, 0.f);
	proj[3] = glue::vec3(0.f, 5.f, 0.f);



	std::array<cv::Mat, 6> shCubemap;
	for (size_t i = 0; i < 6; ++i) {
		shCubemap[i] = cv::Mat(512, 512, CV_32FC3);
	}
	glue::shToCubemap(proj, &shCubemap);
	for (size_t i = 0; i < 6; ++i) {
		cv::imshow("Reprojected " + std::to_string(i), shCubemap[i]);
	}
	cv::waitKey();
	
	return 0;
}
