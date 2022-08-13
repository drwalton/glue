#include "glue/MessageBox.hpp"
#include "glue/Directories.hpp"
#include "glue/Files.hpp"
#ifdef _WIN32
#include <SDL.h>
#endif

int main(int argc, char *argv[])
{
	std::string dir = glue::GLUE_FONT_DIR;
	std::string subDir = glue::makeUniqueSubdirectory(dir);
	glue::showMessageBoxOk("MADE SUBDIR", "Made subdirectory: " + subDir);

	return 0;
}