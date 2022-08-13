#include "glue/PrtCoeffts.hpp"
#include "glue/Exception.hpp"
#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp>

namespace glue {

void saveUncompressedPrtCoeffts(
	const std::string &filename,
	const PrtCoeffts<float> &prtTextures)
{
	static_assert(sizeof(float) == 4,
				  "Can't save & load prt textures, floats not 4 bytes on this platform");
	uint32_t nBands = calcNumSHBands(prtTextures.size());
	uint32_t nChannels = 1;
	uint32_t width32 = prtTextures.width();
	uint32_t height32 = prtTextures.height();
	std::ofstream out(filename, std::ios::out | std::ios::binary);
	out.write((char*)(&nBands), sizeof(uint32_t));
	out.write((char*)(&width32), sizeof(uint32_t));
	out.write((char*)(&height32), sizeof(uint32_t));
	out.write((char*)(&nChannels), sizeof(uint32_t));
	for (size_t i = 0; i < prtTextures.size(); ++i) {
		out.write((char*)(prtTextures[i]),
				  prtTextures.width()*prtTextures.height() * sizeof(float));
	}
	out.close();
}

void saveUncompressedPrtCoeffts(
	const std::string &filename,
	const PrtCoeffts<vec3> &prtTextures)
{
	static_assert(sizeof(float) == 4,
				  "Can't save & load prt textures, floats not 4 bytes on this platform");
	uint32_t nBands = calcNumSHBands(prtTextures.size());
	uint32_t nChannels = 3;
	uint32_t width32 = prtTextures.width();
	uint32_t height32 = prtTextures.height();
	std::ofstream out(filename, std::ios::out | std::ios::binary);
	out.write((char*)(&nBands), sizeof(uint32_t));
	out.write((char*)(&width32), sizeof(uint32_t));
	out.write((char*)(&height32), sizeof(uint32_t));
	out.write((char*)(&nChannels), sizeof(uint32_t));
	for (size_t i = 0; i < prtTextures.size(); ++i) {
		out.write((char*)(prtTextures[i]),
				  prtTextures.width()*prtTextures.height() * sizeof(vec3));
	}
	out.close();
}

template<> PrtCoeffts<float> loadUncompressedPrtCoeffts<float>(const std::string &filename)
{
	uint32_t nBands32, width32, height32, nChannels32;
	std::ifstream in(filename, std::ios::in | std::ios::binary);
	in.seekg(0, in.end);
	std::cout << "File size (bytes): " << in.tellg() << std::endl;
	in.seekg(0, in.beg);
	
	in.read(reinterpret_cast<char*>(&nBands32), sizeof(uint32_t));
	in.read(reinterpret_cast<char*>(&width32), sizeof(uint32_t));
	in.read(reinterpret_cast<char*>(&height32), sizeof(uint32_t));
	in.read(reinterpret_cast<char*>(&nChannels32), sizeof(uint32_t));
	if(nChannels32 != 1) {
		throw FileException("Could not load file: " + filename +
							". Tried to load type float, but file does not have 1 channel!");
	}
	PrtCoeffts<float> coeffts(width32, height32, nBands32);
	for (size_t i = 0; i < coeffts.size(); ++i) {
		in.read((char*)((coeffts)[i]), width32*height32 * sizeof(float));
	}
	return coeffts;
}

template<> PrtCoeffts<vec3> loadUncompressedPrtCoeffts<vec3>(const std::string &filename)
{
	uint32_t nBands32, width32, height32, nChannels32;
	std::ifstream in(filename, std::ios::in | std::ios::binary);
	in.seekg(0, in.end);
	std::cout << "File size (bytes): " << in.tellg() << std::endl;
	in.seekg(0, in.beg);
	
	in.read(reinterpret_cast<char*>(&nBands32), sizeof(uint32_t));
	in.read(reinterpret_cast<char*>(&width32), sizeof(uint32_t));
	in.read(reinterpret_cast<char*>(&height32), sizeof(uint32_t));
	in.read(reinterpret_cast<char*>(&nChannels32), sizeof(uint32_t));
	if(nChannels32 != 3) {
		throw FileException("Could not load file: " + filename +
							". Tried to load type vec3, but file does not have 3 channels!");
	}
	PrtCoeffts<vec3> coeffts(width32, height32, nBands32);
	for (size_t i = 0; i < coeffts.size(); ++i) {
		in.read((char*)((coeffts)[i]), width32*height32 * sizeof(vec3));
	}
	return coeffts;
}

void visualisePrtCoeffts(const PrtCoeffts<float> &prtCoeffts)
{
	float maxPrtCoeffVal = 0.f;
	for (size_t i = 0; i < prtCoeffts.size(); ++i) {
		cv::Mat coeffMat(cv::Size(prtCoeffts.width(), prtCoeffts.height()), CV_32FC1,
			(unsigned char*)(prtCoeffts[i]));
		double min, max;
		cv::minMaxLoc(coeffMat, &min, &max);
		max = fmaxf(-min, max);
		maxPrtCoeffVal = fmaxf(max, maxPrtCoeffVal);
	}
	size_t i = 0;
	for (int l = 0; l < prtCoeffts.nBands(); ++l) {
		for (int m = -l; m <= l; ++m) {
			cv::Mat coeffMat(cv::Size(prtCoeffts.width(), prtCoeffts.height()), CV_32FC1,
				(unsigned char*)(prtCoeffts[i]));
			std::stringstream coeffName;
			coeffName << "l_" << l << "_m_" << m;

			cv::Mat coeffMatR, coeffMatG, coeffMatB;
			coeffMatR = cv::Mat(cv::Size(prtCoeffts.width(), prtCoeffts.height()), CV_8UC1);
			coeffMatR.setTo(0);
			coeffMat.convertTo(coeffMatG, CV_8UC1, 255.f/maxPrtCoeffVal);
			coeffMat *= -1.f;
			coeffMat.convertTo(coeffMatB, CV_8UC1, 255.f/maxPrtCoeffVal);
			coeffMat *= -1.f;
			std::vector<cv::Mat> channels;
			channels.push_back(coeffMatB);
			channels.push_back(coeffMatG);
			channels.push_back(coeffMatR);
			cv::Mat coeffMat8;
			cv::merge(channels, coeffMat8);
			cv::imshow(coeffName.str(), coeffMat8);
			++i;
		}
	}
	cv::waitKey();
}

void visualisePrtCoefftsClickCallback(int event, int x, int y, int flags, void *userData)
{
	if (event == cv::EVENT_LBUTTONDOWN) {
		std::cout << "Clicked at r,c " << y << "," << x << std::endl;


		const PrtCoeffts<vec3> *prtCoeffts =
			reinterpret_cast<const PrtCoeffts<vec3> *>(userData);

		size_t i = 0;
		for (int l = 0; l < prtCoeffts->nBands(); ++l) {
			for (int m = -l; m <= l; ++m) {
				std::cout << "l_" << l << "_m_" << m << ": ";
				vec3 coefft = (*prtCoeffts)[i][y*prtCoeffts->width() + x];
				std::cout << coefft << "\n" << std::endl;
				++i;
			}
		}
	}
}

void visualisePrtCoeffts(const PrtCoeffts<vec3> &prtCoeffts)
{
	float maxPrtCoeffVal = 0.f;
	for (size_t i = 0; i < prtCoeffts.size(); ++i) {
		cv::Mat coeffMat(cv::Size(prtCoeffts.width(), prtCoeffts.height()), CV_32FC3,
			(unsigned char*)(prtCoeffts[i]));
		double min, max;
		cv::minMaxLoc(coeffMat, &min, &max);
		max = fmaxf(-min, max);
		maxPrtCoeffVal = fmaxf(max, maxPrtCoeffVal);
	}
	size_t i = 0;
	for (int l = 0; l < prtCoeffts.nBands(); ++l) {
		for (int m = -l; m <= l; ++m) {
			cv::Mat coeffMat(cv::Size(prtCoeffts.width(), prtCoeffts.height()), CV_32FC3,
				(unsigned char*)(prtCoeffts[i]));
			std::stringstream coeffName;
			coeffName << "l_" << l << "_m_" << m;

			cv::Mat coeffMat8;
			coeffMat.convertTo(coeffMat8, CV_8UC3, 255.f/maxPrtCoeffVal);
			cv::cvtColor(coeffMat8, coeffMat8, cv::COLOR_RGB2BGR);
			cv::imshow(coeffName.str(), coeffMat8);
			cv::setMouseCallback(coeffName.str(), visualisePrtCoefftsClickCallback, 
				const_cast<PrtCoeffts<vec3>*>(&prtCoeffts));
			++i;
		}
	}
	cv::waitKey();
}

}
