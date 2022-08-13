#ifdef _WIN32
#define _USE_MATH_DEFINES
#include <SDL.h>
#endif //_WIN32

#include "glue/Directories.hpp"
#include "glue/Bake.hpp"
#include "glue/PrtCoeffts.hpp"
#include "glue/SphericalHarmonics.hpp"
#include "glue/CubemapLookup.hpp"
#include <FL/Fl_Native_File_Chooser.H>
#include <opencv2/opencv.hpp>
#include <array>

const std::array<std::string, 6> cubemapFilenames = {
	//"negx.jpg", "negy.jpg", "negz.jpg",
	//"posx.jpg", "posy.jpg", "posz.jpg"
	"posx.jpg", "negx.jpg", "posy.jpg",
	"negy.jpg", "posz.jpg", "negz.jpg"
};

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
	std::array<cv::Mat, 6> cubemapImages, reverseCubemapImages;
	for (size_t i = 0; i < 6; ++i) {
		cubemapImages[i] = cv::imread(cubemapDirectory + "/" + cubemapFilenames[i]);
		reverseCubemapImages[i] = cv::Mat(cubemapImages[i].rows, cubemapImages[i].cols, CV_32FC3);
		//cv::flip(cubemapImages[i], cubemapImages[i], 0);
	}

	std::vector<glue::vec3> directions{
		glue::vec3( 1,0,0),
		glue::vec3(-1,0,0),
		glue::vec3(0, 1,0),
		glue::vec3(0,-1,0),
		glue::vec3(0,0, 1),
		glue::vec3(0,0,-1),
		glue::vec3( 1,1,0),
		glue::vec3( 1,0,1)
	};

	for (const glue::vec3 &d : directions) {
		size_t face, row, col;
		glue::dirToCubemapPixel(d, cubemapImages[0].cols, cubemapImages[0].rows, &face, &row, &col);
		glue::vec3 pDir = glue::cubemapPixelToDir(cubemapImages, row, col, face);

		std::cout << "Original Dir\n" << d.normalized() << "\nFace, row, col: " << face << ", " << row << ", " << col << "\n";
		std::cout << "Dir\n" << pDir << "\n\n";
	}
	std::cout << std::endl;
	
	int matWidth = 1024, matHeight = 512;
	cv::Mat sampledImage(cv::Size(matWidth, matHeight), CV_32FC3);
	
	for(int r = 0; r < sampledImage.rows; ++r) {
		for(int c = 0; c < sampledImage.cols; ++c) {
			float theta = -float(r) * float(M_PI) / float(sampledImage.rows);
			theta += float(M_PI_2);
			float phi   = float(c) * 2.f * float(M_PI) / float(sampledImage.cols);
			glue::vec3 dir = glue::vec3(cosf(theta)*cosf(phi), sinf(theta),  cosf(theta)*sinf(phi));
			glue::vec3 sample = glue::sampleCubemap(cubemapImages, dir);
			sampledImage.at<cv::Vec3f>(r,c) = cv::Vec3f(sample.x(), sample.y(), sample.z());
		}
	}
	
	cv::imshow("Sampled image (Mercator)", sampledImage);
	
	for(size_t face = 0; face < 6; ++face) {
		for(size_t row = 0; row < reverseCubemapImages[face].rows; ++row) {
			for(size_t col = 0; col < reverseCubemapImages[face].cols; ++col) {
				glue::vec3 dir = glue::cubemapPixelToDir(reverseCubemapImages, row, col, face);
    			glue::vec3 sample = glue::sampleCubemap(cubemapImages, dir);
				reverseCubemapImages[face].at<cv::Vec3f>(row, col) = cv::Vec3f(sample.x(), sample.y(), sample.z());
			}
		}
		cv::imshow("Reverse Cubemap " + std::to_string(face), reverseCubemapImages[face]);
	}
	cv::waitKey();
	
	return 0;
}
