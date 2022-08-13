#include "glue/Bake.hpp"
#include "glue/Directories.hpp"
#include "glue/ModelLoader.hpp"
#include <memory>
#include <opencv2/opencv.hpp>
#include <FL/Fl_Native_File_Chooser.H>
#include <SDL.h>

//This application is designed to take two PRT textures, and produce one as output.
//The inputs should be as follows:
//The first, A, should be a normal diffuse PRT texture for the object on a plane.
//The second, B, should be a normal diffuse PRT texture for just the plane, without the object.
//Note: the plane should have the same texture coords in both cases.
//The output is a differential PRT texture. This will be identical to A, except in the region
//occupied by the plane. Here, the texture will contain A-B (that is, the influence the object has on the plane).

int main(int argc, char *argv[])
{
	std::string texAFilename;
	std::string texBFilename;
	std::string texOutFilename;
	
	
	if (argc >= 4) {
		texAFilename = argv[1];
		texBFilename = argv[2];
		texOutFilename = argv[3];
	} else {
		Fl_Native_File_Chooser chooser(Fl_Native_File_Chooser::BROWSE_FILE);
		chooser.directory(glue::GLUE_MODEL_DIR.c_str());
		chooser.title("Choose PRT Texture A (object and plane)");
		if (chooser.show() != 0) {
			return 1;
		}
		texAFilename = chooser.filename();
		chooser.title("Choose PRT Texture B (just plane)");
		if (chooser.show() != 0) {
			return 1;
		}
		texBFilename = chooser.filename();
		chooser.title("Choose Output PRT Texture Filename");
		if (chooser.show() != 0) {
			return 1;
		}
		texOutFilename = chooser.filename();
	}
	
	glue::PrtCoeffts<float> prtA = glue::loadUncompressedPrtCoeffts<float>(texAFilename);
	glue::PrtCoeffts<float> prtB = glue::loadUncompressedPrtCoeffts<float>(texBFilename);
	
	if(prtA.nBands() != prtB.nBands()) {
		throw std::runtime_error("Supplied inputs have different numbers of bands!");
	}
	if(prtA.width() != prtB.width() || prtA.height() != prtB.height()) {
		throw std::runtime_error("Supplied inputs have different dimensions!");
	}
	
	glue::PrtCoeffts<float> prtOut(prtA.width(), prtA.height(), prtA.nBands());

	for(size_t i = 0; i < prtA.size(); ++i) {
		for(size_t r = 0; r < prtA.height(); ++r) {
			for(size_t c = 0; c < prtA.width(); ++c) {
				prtOut[i][r*prtA.width() + c] =
					prtA[i][r*prtA.width() + c] - prtB[i][r*prtA.width() + c];
			}
		}
	}

	glue::saveUncompressedPrtCoeffts(texOutFilename, prtOut);

	cv::waitKey(1);

	//Let OS cleanup stuff.

	return 0;
}
