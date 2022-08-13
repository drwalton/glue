#include "glue/Bake.hpp"
#include "glue/Directories.hpp"
#include "glue/ModelLoader.hpp"
#include <memory>
#include <opencv2/opencv.hpp>
#include <FL/Fl_Native_File_Chooser.H>
#include <SDL.h>

//This application is designed to take two meshes with associated textures, and move the texture coordinates
// so that they fit in different halves of the square. Three versions of the mesh are then saved - one with both meshes,
// two others with just one of the pair. All three mesh files have the same texture coordinates.
int main(int argc, char *argv[])
{
	std::string realMeshFilename;
	std::string virtualMeshFilename;
	std::string meshOutFilename;
	
	
	if (argc >= 4) {
		realMeshFilename = argv[1];
		virtualMeshFilename = argv[2];
		meshOutFilename = argv[3];
	} else {
		Fl_Native_File_Chooser chooser(Fl_Native_File_Chooser::BROWSE_FILE);
		chooser.directory(glue::GLUE_MODEL_DIR.c_str());
		chooser.title("Choose real object mesh");
		if (chooser.show() != 0) {
			return 1;
		}
		realMeshFilename = chooser.filename();
		chooser.title("Choose virtual object mesh");
		if (chooser.show() != 0) {
			return 1;
		}
		virtualMeshFilename = chooser.filename();
		chooser.title("Choose Output Filename");
		if (chooser.show() != 0) {
			return 1;
		}
		meshOutFilename = chooser.filename();
	}
	
	glue::ModelLoader realModelLoader(realMeshFilename);
	glue::ModelLoader virtModelLoader(virtualMeshFilename);
	glue::ModelLoader realAndVirtOutput, realOnlyOutput, virtOnlyOutput;
	
	std::vector<glue::vec2> convertedVirtTexCoords, convertedRealTexCoords;
	for (auto &v : realModelLoader.texCoords()) {
		convertedRealTexCoords.push_back(glue::vec2(v.x(), v.y() * 0.5f));
	}
	for (auto &v : virtModelLoader.texCoords()) {
		convertedVirtTexCoords.push_back(glue::vec2(v.x(), 0.5f + v.y() * 0.5f));
	}

	realOnlyOutput.indices() = realModelLoader.indices();
	realOnlyOutput.vertices() = realModelLoader.vertices();
	realOnlyOutput.normals() = realModelLoader.normals();
	realOnlyOutput.texCoords() = convertedRealTexCoords;

	virtOnlyOutput.indices() = virtModelLoader.indices();
	virtOnlyOutput.vertices() = virtModelLoader.vertices();
	virtOnlyOutput.normals() = virtModelLoader.normals();
	virtOnlyOutput.texCoords() = convertedVirtTexCoords;

	for (auto &v : realModelLoader.vertices()) {
		realAndVirtOutput.vertices().push_back(v);
	}
	for (auto &v : virtModelLoader.vertices()) {
		realAndVirtOutput.vertices().push_back(v);
	}
	for (auto &n : realModelLoader.normals()) {
		realAndVirtOutput.normals().push_back(n);
	}
	for (auto &n : virtModelLoader.normals()) {
		realAndVirtOutput.normals().push_back(n);
	}
	for (auto &v : convertedRealTexCoords) {
		realAndVirtOutput.texCoords().push_back(v);
	}
	for (auto &v : convertedVirtTexCoords) {
		realAndVirtOutput.texCoords().push_back(v);
	}
	size_t offset = realModelLoader.vertices().size();
	for (auto &i : realModelLoader.indices()) {
		realAndVirtOutput.indices().push_back(i);
	}
	for (auto &i : virtModelLoader.indices()) {
		realAndVirtOutput.indices().push_back(i + offset);
	}

	realOnlyOutput.saveFile(meshOutFilename + "_real.obj");
	virtOnlyOutput.saveFile(meshOutFilename + "_virt.obj");
	realAndVirtOutput.saveFile(meshOutFilename + "_real_virt.obj");


	cv::waitKey(1);

	//Let OS cleanup stuff.

	return 0;
}
