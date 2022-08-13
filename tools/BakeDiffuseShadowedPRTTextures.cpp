#include "glue/Bake.hpp"
#include "glue/Directories.hpp"
#include "glue/ModelLoader.hpp"
#include <memory>
#include <opencv2/opencv.hpp>
#include <FL/Fl_Native_File_Chooser.H>
#include <SDL.h>

int main(int argc, char *argv[])
{
	size_t width = 512, height = 512, nSamples = 10000, nBands = 5;
	std::string modelFilename;
	if (argc >= 2) {
		modelFilename = argv[1];
	} else {
		Fl_Native_File_Chooser chooser(Fl_Native_File_Chooser::BROWSE_FILE);
		chooser.directory(glue::GLUE_MODEL_DIR.c_str());
		if (chooser.show() != 0) {
			return 1;
		}
		modelFilename = chooser.filename();
	}
	if (argc >= 4) {
		width = atoi(argv[2]);
		height = atoi(argv[3]);
	}
	if (argc >= 5) {
		nSamples = atoi(argv[4]);
	}
	if (argc >= 6) {
		nBands = atoi(argv[5]);
	}

	std::cout << "Baking Diffuse Shadowed PRT for model \"" << modelFilename
		<< "\"\nWidth: " << width << ", Height: " << height
		<< "\nNum. Samples: " << nSamples
		<< "\nNum. SH Bands: " << nBands << std::endl;

	glue::ModelLoader loader(modelFilename);
	std::unique_ptr<glue::vec4[]> worldPosImage(new glue::vec4[width*height]);
	std::unique_ptr<glue::vec4[]> normalImage(new glue::vec4[width*height]);
	glue::renderWorldPosAndNormals(loader.vertices(), loader.normals(),
		loader.texCoords(), loader.indices(),
		worldPosImage.get(), normalImage.get(), width, height);

	cv::Mat worldPosMat(cv::Size(int(width), int(height)), CV_32FC4,
		reinterpret_cast<unsigned char*>(worldPosImage.get()));
	cv::Mat normalMat(cv::Size(int(width), int(height)), CV_32FC4,
		reinterpret_cast<unsigned char*>(normalImage.get()));
	cv::imshow("World Pos", worldPosMat);
	cv::imshow("Normals", normalMat);
	cv::waitKey(1);

	glue::PrtCoeffts<float> prtCoeffts = glue::bakeDiffuseShadowedPrtCoeffts(
		loader.vertices(), loader.normals(), loader.texCoords(),
		loader.indices(),
		worldPosImage.get(), normalImage.get(),
		width, height,
		nBands,
		nSamples);

	glue::saveUncompressedPrtCoeffts(
		modelFilename + ".s_" +
		std::to_string(nSamples) + "_b_" + 
		std::to_string(nBands) + "_w_" + 
		std::to_string(width) + ".prt",
		prtCoeffts);

	cv::waitKey(1);

	//Let OS cleanup stuff.

	return 0;
}
