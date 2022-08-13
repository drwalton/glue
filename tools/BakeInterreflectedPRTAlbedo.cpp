#include "glue/PrtBaker.hpp"
#include "glue/Directories.hpp"
#include "glue/ModelLoader.hpp"
#include "glue/PrtCoeffts.hpp"
#include "boost/property_tree/json_parser.hpp"
#include <memory>
#include <opencv2/opencv.hpp>
#include <FL/Fl_Native_File_Chooser.H>
#include <SDL.h>

int main(int argc, char *argv[])
{
	if (argc < 2) {
		std::cout << "Usage $ bake [configFilename]" << std::endl;
		return 1;
	}
	boost::property_tree::ptree config;
	boost::property_tree::json_parser::read_json(argv[1], config);

	size_t width = config.get<int>("width");
	size_t height = config.get<int>("height");
	size_t nSamples = config.get<int>("nSamples");
	size_t nBands = config.get<int>("nBands");
	size_t nBounces = config.get<int>("nBounces");
	std::string modelFilename = config.get<std::string>("modelFilename");
	std::string albedoTexFilename = config.get<std::string>("albedoTexFilename");
	bool useSpecularMesh = config.get<bool>("useSpecularMesh");
	bool useDifferentialRendering = config.get<bool>("useDifferentialRendering");

	std::string specularMeshFilename;
	if (useSpecularMesh) {
		specularMeshFilename = config.get<std::string>("specularMeshFilename");
	}
	std::string differentialRealMeshFile;
	if (useDifferentialRendering) {
		differentialRealMeshFile = config.get<std::string>("differentialRealMeshFile");
	}

	std::string outTexFilename, outTexFilenameReal, outTexFilenameDifferential;

	std::cout << "Baking Diffuse Shadowed PRT for model \"" << modelFilename
		<< "\"\nWidth: " << width << ", Height: " << height
		<< "\nNum. Samples: " << nSamples
		<< "\nNum. SH Bands: " << nBands << std::endl;

	{
		glue::PrtBaker prtBaker(nBands, nSamples);
		glue::ModelLoader loader(modelFilename);
		if (!loader.hasNormals() || !loader.hasTexCoords()) {
			std::cout << "ERROR Loaded model lacks tex coords or normals. Cannot use." << std::endl;
			return 2;
		}
		cv::Mat albedoTexMat = cv::imread(albedoTexFilename);
		cv::flip(albedoTexMat, albedoTexMat, 0);
		cv::cvtColor(albedoTexMat, albedoTexMat, cv::COLOR_BGR2RGB);
		if (albedoTexMat.cols != width || albedoTexMat.rows != height) {
			std::cout << "ERROR Width, height must correspond to albedo texture width, height." << std::endl;
			return 3;
		}
		cv::Mat albedoTexMatFloat;
		albedoTexMat.convertTo(albedoTexMatFloat, CV_32FC3, 1.f / 255.f);
		prtBaker.setupMesh(loader.vertices(), loader.normals(),
			loader.texCoords(), loader.indices(), width, height);

		glue::ModelLoader specularLoader;
		if (useSpecularMesh) {
			specularLoader.loadFile(specularMeshFilename);
			prtBaker.setupSpecularMesh(specularLoader.vertices(), specularLoader.normals(), specularLoader.indices());
		}

		glue::PrtCoeffts<glue::vec3> bakedCoeffts(width, height, nBands);
		if (!useSpecularMesh) {
			prtBaker.bakeDiffuseInterreflectedPrt(nBounces,
				&bakedCoeffts, reinterpret_cast<glue::vec3*>(albedoTexMatFloat.data));
		} else {
			prtBaker.bakeDiffuseInterreflectedPrtSpecular(nBounces,
				&bakedCoeffts, reinterpret_cast<glue::vec3*>(albedoTexMatFloat.data));
		}

		outTexFilename =
			modelFilename + ".s_" +
			std::to_string(nSamples) + "_b_" +
			std::to_string(nBands) + "_w_" +
			std::to_string(width) + "_lb_" +
			std::to_string(nBounces) + ".prtc";

		glue::saveUncompressedPrtCoeffts(
			outTexFilename, bakedCoeffts);

		cv::waitKey(1);
	}

	if (useDifferentialRendering) {
		{
			glue::PrtBaker prtBaker(nBands, nSamples);
			glue::ModelLoader loader(differentialRealMeshFile);
			cv::Mat albedoTexMat = cv::imread(albedoTexFilename);
			cv::flip(albedoTexMat, albedoTexMat, 0);
			cv::cvtColor(albedoTexMat, albedoTexMat, cv::COLOR_BGR2RGB);
			if (albedoTexMat.cols != width || albedoTexMat.rows != height) {
				std::cout << "ERROR Width, height must correspond to albedo texture width, height." << std::endl;
				return 3;
			}
			cv::Mat albedoTexMatFloat;
			albedoTexMat.convertTo(albedoTexMatFloat, CV_32FC3, 1.f / 255.f);
			prtBaker.setupMesh(loader.vertices(), loader.normals(),
				loader.texCoords(), loader.indices(), width, height);
			glue::PrtCoeffts<glue::vec3> bakedCoeffts(width, height, nBands);
			prtBaker.bakeDiffuseInterreflectedPrt(nBounces,
				&bakedCoeffts, reinterpret_cast<glue::vec3*>(albedoTexMatFloat.data));

			outTexFilenameReal = modelFilename + "_real.s_" +
				std::to_string(nSamples) + "_b_" +
				std::to_string(nBands) + "_w_" +
				std::to_string(width) + "_lb_" +
				std::to_string(nBounces) + ".prtc";
			outTexFilenameDifferential = modelFilename + "_differential.s_" +
				std::to_string(nSamples) + "_b_" +
				std::to_string(nBands) + "_w_" +
				std::to_string(width) + "_lb_" +
				std::to_string(nBounces) + ".prtc";
			glue::saveUncompressedPrtCoeffts(
				outTexFilenameReal, bakedCoeffts);
		}
		glue::PrtCoeffts<glue::vec3> prtA = glue::loadUncompressedPrtCoeffts<glue::vec3>(outTexFilename);
		glue::PrtCoeffts<glue::vec3> prtB = glue::loadUncompressedPrtCoeffts<glue::vec3>(outTexFilenameReal);
		glue::PrtCoeffts<glue::vec3> prtOut(prtA.width(), prtA.height(), prtA.nBands());

		for (size_t i = 0; i < prtA.size(); ++i) {
			for (size_t r = 0; r < prtA.height(); ++r) {
				for (size_t c = 0; c < prtA.width(); ++c) {
					prtOut[i][r*prtA.width() + c] =
						prtA[i][r*prtA.width() + c] - prtB[i][r*prtA.width() + c];
				}
			}
		}

		glue::saveUncompressedPrtCoeffts(outTexFilenameDifferential, prtOut);
	}

	return 0;
}
