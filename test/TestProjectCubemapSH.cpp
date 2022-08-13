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
const size_t nCubemapSamples = 100;
const size_t nBands = 6;

int main(int argc, char *argv[])
{
	std::string modelFilename, textureFilename, cubemapDirectory;
	Fl_Native_File_Chooser chooser(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
	chooser.directory(glue::GLUE_MODEL_DIR.c_str());
	chooser.title("Load Cubemap Image Folder");
	if (chooser.show() != 0) {
		return 1;
	}
	cubemapDirectory = chooser.filename();
	glue::SHProjection3 cubemapSH, cubemapSHAllPix;
	std::array<cv::Mat, 6> cubemapImages;
	for (size_t i = 0; i < 6; ++i) {
		cubemapImages[i] = cv::imread(cubemapDirectory + "/" + cubemapFilenames[i]);
	}
	cubemapSH = glue::projectCubemap(cubemapImages, nCubemapSamples, nBands);
	cubemapSHAllPix = glue::projectCubemap(cubemapImages, nBands);
	
	std::cout << "Projection: " << cubemapSH[0];
	for(size_t i = 1; i < cubemapSH.size(); ++i) {
		std::cout << ", " << cubemapSH[i];
	}
	
	std::cout << "\n\nProjection2: " << cubemapSH[0];
	for(size_t i = 1; i < cubemapSHAllPix.size(); ++i) {
		std::cout << ", " << cubemapSHAllPix[i];
	}

	std::array<cv::Mat, 6> shCubemap;
	for (size_t i = 0; i < 6; ++i) {
		shCubemap[i] = cv::Mat(cubemapImages[i].rows, cubemapImages[i].cols, CV_32FC3);
	}
	glue::shToCubemap(cubemapSH, &shCubemap);
	for (size_t i = 0; i < 6; ++i) {
		cv::imshow("Reprojected " + cubemapFilenames[i], shCubemap[i]);
	}
	
	glue::shToCubemap(cubemapSHAllPix, &shCubemap);
	for (size_t i = 0; i < 6; ++i) {
		cv::imshow("Reprojected2 " + cubemapFilenames[i], shCubemap[i]);
	}
	cv::waitKey();
	
	chooser.type(Fl_Native_File_Chooser::BROWSE_FILE);
	chooser.title("Load PRT Textures");
	if (chooser.show() != 0) {
		return 1;
	}
	glue::PrtCoeffts<float> prtCoeffts =
		glue::loadUncompressedPrtCoeffts<float>(chooser.filename());
	glue::viewPrtCoefftsUnderLighting(prtCoeffts, cubemapSH);
	
	return 0;
}
