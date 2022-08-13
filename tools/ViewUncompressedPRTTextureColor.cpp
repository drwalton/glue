#include "glue/Directories.hpp"
#include "glue/Bake.hpp"
#include <FL/Fl_Native_File_Chooser.H>
#include <opencv2/opencv.hpp>
#include <SDL.h>

int main(int argc, char *argv[])
{
	std::string filename;
	if (argc > 2) {
		filename = argv[1];
	}
	else {
		Fl_Native_File_Chooser chooser(Fl_Native_File_Chooser::BROWSE_FILE);
		chooser.directory(glue::GLUE_MODEL_DIR.c_str());
		if (chooser.show() != 0) {
			return 1;
		}
		filename = chooser.filename();
	}
	glue::PrtCoeffts<glue::vec3> prtCoeffts =
		glue::loadUncompressedPrtCoeffts<glue::vec3>(filename);
	glue::visualisePrtCoeffts(prtCoeffts);

	return 0;
}
