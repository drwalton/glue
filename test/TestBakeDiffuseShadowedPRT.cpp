#include "glue/Bake.hpp"
#include "glue/Directories.hpp"
#include "glue/ModelLoader.hpp"
#include <memory>
#include <opencv2/opencv.hpp>
#ifdef _WIN32
#include <SDL.h>
#endif

int main(int argc, char *argv[])
{
	size_t width = 512, height = 512, nSamples = 100, nBands = 4;
	std::unique_ptr<glue::vec4[]> worldPosImage(new glue::vec4[width*height]);
	std::unique_ptr<glue::vec4[]> normalImage(new glue::vec4[width*height]);
	glue::ModelLoader loader(glue::GLUE_MODEL_DIR + "monkey.obj");
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
		width, height, nBands, nSamples);

	//glue::visualisePRTTextures(prtTextures, width, height);

	glue::saveUncompressedPrtCoeffts(
		glue::GLUE_MODEL_DIR + "bunnyDiffusePRT_s_" + 
		std::to_string(nSamples) + "_b_" + 
		std::to_string(nBands) + "_w_" + 
		std::to_string(width) + ".prt",
		prtCoeffts);

	cv::Mat worldPos8, normals8;
	worldPosMat.convertTo(worldPos8, CV_8UC3, 255.f);
	normalMat.convertTo(normals8, CV_8UC3, 255.f);
	cv::imwrite(glue::GLUE_MODEL_DIR + "bunnyWorldPos.png", worldPos8);
	cv::imwrite(glue::GLUE_MODEL_DIR + "bunnyNormals.png", normals8);

	cv::waitKey(1);

	//Let OS cleanup stuff.

	return 0;
}
