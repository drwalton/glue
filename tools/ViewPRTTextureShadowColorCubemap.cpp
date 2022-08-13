#include "glue/Directories.hpp"
#include "glue/Bake.hpp"
#include "glue/SphericalHarmonics.hpp"
#include "glue/FullScreenQuad.hpp"
#include "glue/GLWindow.hpp"
#include "glue/RotateViewer.hpp"
#include "glue/ShaderProgram.hpp"
#include "glue/ModelLoader.hpp"
#include "glue/PrtMeshColorShadowed.hpp"
#include "glue/Matrices.hpp"
#include "glue/ArrayTexture.hpp"
#include "glue/Texture.hpp"
#include "glue/SphericalPlot.hpp"
#include <FL/Fl_Native_File_Chooser.H>
#include <opencv2/opencv.hpp>
#include <array>
#include <Eigen/Dense>

const std::array<std::string, 6> cubemapFilenames = {
	"posx.jpg", "negx.jpg", "posy.jpg",
	"negy.jpg", "posz.jpg", "negz.jpg"
};
const size_t nCubemapSamples = 1000;
const glue::vec3 albedo(0, 0.75, 0.75);
float shadowPower = 1.7f;

int main(int argc, char *argv[])	
{
	std::string modelFilename, modelShadowFilename, 
		modelSurfaceFilename = glue::GLUE_MODEL_DIR + "BigPlane.obj", textureFilename, 
		surfaceImageFilename = glue::GLUE_MODEL_DIR + "surfaceImage.jpg", cubemapDirectory;
	if (argc == 5) {
		modelFilename = argv[1];
		modelShadowFilename = argv[2];
		textureFilename = argv[3];
		cubemapDirectory = argv[4];
	}
	else {
		Fl_Native_File_Chooser chooser(Fl_Native_File_Chooser::BROWSE_FILE);
		chooser.directory(glue::GLUE_MODEL_DIR.c_str());
		chooser.title("Load Model File");
		if (chooser.show() != 0) {
			return 1;
		}
		modelFilename = chooser.filename();
		chooser.title("Load Shadow Model File");
		if (chooser.show() != 0) {
			return 1;
		}
		modelShadowFilename = chooser.filename();
		chooser.title("Load PRT Texture File");
		if (chooser.show() != 0) {
			return 1;
		}
		textureFilename = chooser.filename();
		chooser.type(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
		chooser.title("Load Cubemap Image Folder");
		if (chooser.show() != 0) {
			return 1;
		}
		cubemapDirectory = chooser.filename();
	}
	glue::GLWindow win("PRT Preview", 1280, 960);
	win.updateText("Test");
	glue::PrtMeshColorShadowed prtMesh(modelFilename, textureFilename, modelShadowFilename);
	//prtMesh.monoColor(glue::vec3(0.f, 1.f, 1.f));
	cv::Mat surfaceImage = cv::imread(surfaceImageFilename);
	glue::Texture surfaceTex(GL_TEXTURE_2D, GL_RGB8, surfaceImage.cols, surfaceImage.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, surfaceImage.data);

	size_t nBands = prtMesh.nBands();
	glue::SHProjection3 cubemapSH;
	
	std::array<cv::Mat, 6> cubemapImages;
	for (size_t i = 0; i < 6; ++i) {
		cubemapImages[i] = cv::imread(cubemapDirectory + "/" + cubemapFilenames[i]);
		//cv::imshow("cubemap", cubemapImages[i]);
		//cv::waitKey();
	}
	cubemapSH = glue::projectCubemap(cubemapImages, nCubemapSamples, nBands);
	glue::SphericalPlot cubemapSHPlot([&cubemapSH](float theta, float phi) {
			glue::vec3 val = glue::evaluateSH(cubemapSH, theta, phi);
			return 2.f * val.sum();
		}, 40);
	cubemapSHPlot.modelToWorld(glue::translateMat4(glue::vec3(4.f, 1.f, 0.f)));
	
	std::cout << "Projection: " << cubemapSH[0];
	for(size_t i = 1; i < cubemapSH.size(); ++i) {
		std::cout << ", " << cubemapSH[i];
	}
	std::cout << std::endl;
	
	glue::RotateViewer viewer(&win);
	GLuint cubemapTex;
	glGenTextures(1, &cubemapTex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTex);
	for(int face = 0; face < 6; ++face) {
		cv::Mat im;
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
			0, GL_RGB8, cubemapImages[face].cols, cubemapImages[face].rows,
			0, GL_BGR, GL_UNSIGNED_BYTE, cubemapImages[face].data);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	
	glue::ShaderProgram viewEnvMapProgram(std::vector<std::string> {
		glue::GLUE_SHADER_DIR + "FullScreenTex.vert",
		glue::GLUE_SHADER_DIR + "FullScreenTex_ViewEnvMap.frag"
	});
	glue::ShaderProgram drawSurfaceProgram(std::vector<std::string> {
		glue::GLUE_SHADER_DIR + "TexturedMesh.vert",
		glue::GLUE_SHADER_DIR + "TexturedMesh.frag"
	});
	
	viewEnvMapProgram.setUniform("cubemap", 0);
	drawSurfaceProgram.setUniform("tex", 0);
	
	glue::Mesh surfaceMesh;
	{
		glue::ModelLoader loader(modelSurfaceFilename);
		surfaceMesh.fromModelLoader(loader);
	}
	surfaceMesh.shaderProgram(&drawSurfaceProgram);
	SDL_Event event;
	bool running = true;
	glClearColor(0, 1, 0, 1);
	
	while(running) {
		while(SDL_PollEvent(&event)) {
			viewer.processEvent(event);
			if(glue::GLWindow::eventIsQuit(event)) {
				running = false;
			}
			if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.sym == SDLK_0 ||
				    event.key.keysym.sym == SDLK_9) {
					if (event.key.keysym.sym == SDLK_0) {
						shadowPower += 0.1f;
					} else {
						shadowPower -= 0.1f;
					}
					std::cout << "Shadow power: " << shadowPower << std::endl;
					prtMesh.realSurfaceAlbedo(glue::vec3(shadowPower, shadowPower, shadowPower));
				}
			}
		}
		
		//Draw environment map.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
    	glActiveTexture(GL_TEXTURE0 + 0);
    	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTex);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glue::mat3 invRot = viewer.rotation().inverse();
		viewEnvMapProgram.setUniform("invRotMat", invRot);
		glue::FullScreenQuad::getInstance().render(viewEnvMapProgram);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		//Draw surface.
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		surfaceTex.bindToImageUnit(0);
		surfaceMesh.render();
		
		prtMesh.lightingCoeffts(cubemapSH);
		prtMesh.render();

		cubemapSHPlot.render();

		win.drawText(30, 30);
		glDepthMask(GL_TRUE);
		
		win.swapBuffers();
		
		SDL_Delay(30);
	}

	return 0;
}
