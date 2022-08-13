#include "glue/Directories.hpp"
#include "glue/Bake.hpp"
#include "glue/SphericalHarmonics.hpp"
#include "glue/FullScreenQuad.hpp"
#include "glue/GLWindow.hpp"
#include "glue/RotateViewer.hpp"
#include "glue/ShaderProgram.hpp"
#include "glue/Matrices.hpp"
#include "glue/ModelLoader.hpp"
#include "glue/Mesh.hpp"
#include "glue/ArrayTexture.hpp"
#include "glue/SphericalPlot.hpp"
#include <FL/Fl_Native_File_Chooser.H>
#include <opencv2/opencv.hpp>
#include <array>
#include <Eigen/Dense>

const std::array<std::string, 6> cubemapFilenames = {
	"posx.jpg", "negx.jpg", "posy.jpg",
	"negy.jpg", "posz.jpg", "negz.jpg"
};
const size_t nCubemapSamples = 100;
const glue::vec3 albedo(0, 0.75, 0.75);
const float shBandMagnitude = .5f;
size_t nonZeroCoefft = 0;

int main(int argc, char *argv[])	
{
	glue::GLWindow win("PRT Preview", 1280, 960);
	glue::RotateViewer viewer(&win);
	win.updateText("Test");

	size_t nBands = 5;
	glue::SHProjection3 shCoeffts(glue::calcNumSHCoeffts(5));
	shCoeffts[0] = glue::vec3(shBandMagnitude, shBandMagnitude, shBandMagnitude);
	for (size_t i = 1; i < shCoeffts.size(); ++i) {
		shCoeffts[i] = glue::vec3::Zero();
	}
	
	glue::SphericalPlot shPlot([&shCoeffts](float theta, float phi) {
			glue::vec3 val = glue::evaluateSH(shCoeffts, theta, phi);
			return 5.f * val.x();
		}, 40);
	shPlot.modelToWorld(glue::translateMat4(glue::vec3(3.f, 2.f, 0.f)));

	glue::SphericalPlot shPlot2([&shCoeffts](float theta, float phi) {
			glue::vec3 dir = glue::sphericalAngleToDir(theta, phi);
			glue::vec3 val = glue::evaluateSH(shCoeffts, dir);
			return 5.f * val.x();
		}, 40);
	shPlot2.modelToWorld(glue::translateMat4(glue::vec3(-3.f, -2.f, 0.f)));

	/*
	std::cout << "Projection: " << cubemapSH[0];
	for(size_t i = 1; i < cubemapSH.size(); ++i) {
		std::cout << ", " << cubemapSH[i];
	}
	std::cout << std::endl;
	*/
	
	//glue::visualisePrtCoeffts(prtTextures);

	//glue::PrtCoeffts<float> prtTextures =
	//	glue::loadUncompressedPrtCoeffts<float>(textureFilename);
	//glue::viewPrtCoefftsUnderLighting(prtTextures, cubemapSH);
	

	SDL_Event event;
	bool running = true;
	glClearColor(0.1, 0.1, 0.1, 1);
	
	while(running) {
		while(SDL_PollEvent(&event)) {
			viewer.processEvent(event);
			if(glue::GLWindow::eventIsQuit(event)) {
				running = false;
			}

			if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.sym == SDLK_SPACE) {
					shCoeffts[nonZeroCoefft] = glue::vec3::Zero();
					nonZeroCoefft = (nonZeroCoefft + 1) % shCoeffts.size();
					shCoeffts[nonZeroCoefft] = glue::vec3(shBandMagnitude, shBandMagnitude, shBandMagnitude);
					shPlot.replot([&shCoeffts](float theta, float phi) {
						glue::vec3 val = glue::evaluateSH(shCoeffts, theta, phi);
						return 5.f * val.x(); 
					});
					shPlot2.replot([&shCoeffts](float theta, float phi) {
						glue::vec3 dir = glue::sphericalAngleToDir(theta, phi);
						glue::vec3 val = glue::evaluateSH(shCoeffts, dir);
						return 5.f * val.x();
					});
					
					win.setTitle("Coefft " + std::to_string(nonZeroCoefft));
				}
			}
		}
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		//Draw mesh.
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);

		//glDisable(GL_DEPTH_TEST);
		//glDisable(GL_CULL_FACE);
		shPlot.render();
		shPlot2.render();

		win.drawText(30, 30);
		glDepthMask(GL_TRUE);
		
		win.swapBuffers();
		
		SDL_Delay(30);
	}

	return 0;
}
