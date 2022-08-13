#include "glue/Bake.hpp"
#include "glue/Directories.hpp"
#include "glue/ModelLoader.hpp"
#include <memory>
#include <opencv2/opencv.hpp>
#include <SDL.h>

int main(int argc, char *argv[])
{
	size_t width = 512, height = 512, nSamples = 1000;
	std::unique_ptr<glue::vec4[]> worldPosImage(new glue::vec4[width*height]);
	std::unique_ptr<glue::vec4[]> normalImage(new glue::vec4[width*height]);
	glue::ModelLoader loader(glue::GLUE_MODEL_DIR + "bunny.obj");
	glue::renderWorldPosAndNormals(loader.vertices(), loader.normals(),
		loader.texCoords(), loader.indices(),
		worldPosImage.get(), normalImage.get(), width, height);

	cv::Mat worldPosMat(cv::Size(width, height), CV_32FC4, 
		reinterpret_cast<unsigned char*>(worldPosImage.get()));
	cv::Mat normalMat(cv::Size(width, height), CV_32FC4, 
		reinterpret_cast<unsigned char*>(normalImage.get()));
	cv::imshow("World Pos", worldPosMat);
	cv::imshow("Normals", normalMat);
	cv::waitKey(1);

	std::unique_ptr<float[]> aoImage(new float[width*height]);
	glue::bakeAmbientOcclusionTexture(
		loader.vertices(), loader.normals(), loader.texCoords(), 
		loader.indices(), aoImage.get(), 
		worldPosImage.get(), normalImage.get(), 
		width, height,
		nSamples);

	cv::Mat aoMat(cv::Size(width, height), CV_32FC1,
		reinterpret_cast<unsigned char*>(aoImage.get()));

	cv::Mat aoMat8, worldPos8, normals8;
	aoMat.convertTo(aoMat8, CV_8UC1, 255.f);
	worldPosMat.convertTo(worldPos8, CV_8UC3, 255.f);
	normalMat.convertTo(normals8, CV_8UC3, 255.f);
	cv::imwrite(glue::GLUE_MODEL_DIR + "bunnyAO.png", aoMat8);
	cv::imwrite(glue::GLUE_MODEL_DIR + "bunnyWorldPos.png", worldPos8);
	cv::imwrite(glue::GLUE_MODEL_DIR + "bunnyNormals.png", normals8);

	cv::imshow("Ambient Occlusion", aoMat);
	cv::waitKey();

	return 0;
}
