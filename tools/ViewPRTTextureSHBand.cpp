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
	std::string modelFilename, textureFilename;
	if (argc == 3) {
		modelFilename = argv[1];
		textureFilename = argv[2];
	}
	else {
		Fl_Native_File_Chooser chooser(Fl_Native_File_Chooser::BROWSE_FILE);
		chooser.directory(glue::GLUE_MODEL_DIR.c_str());
		chooser.title("Load Model File");
		if (chooser.show() != 0) {
			return 1;
		}
		modelFilename = chooser.filename();
		chooser.title("Load PRT Texture File");
		if (chooser.show() != 0) {
			return 1;
		}
		textureFilename = chooser.filename();
	}
	glue::GLWindow win("PRT Preview", 1280, 960);
	glue::RotateViewer viewer(&win);
	win.updateText("Test");
	glue::ArrayTexture prtCoefftArrTex(textureFilename);

	size_t nBands = glue::calcNumSHBands(prtCoefftArrTex.nLayers());
	glue::SHProjection3 shCoeffts(prtCoefftArrTex.nLayers());
	shCoeffts[0] = glue::vec3(shBandMagnitude, shBandMagnitude, shBandMagnitude);
	for (size_t i = 1; i < shCoeffts.size(); ++i) {
		shCoeffts[i] = glue::vec3::Zero();
	}
	
	glue::SphericalPlot shPlot([&shCoeffts](float theta, float phi) {
			glue::vec3 val = glue::evaluateSH(shCoeffts, theta, phi);
			return 5.f * val.x();
		}, 40);
	shPlot.modelToWorld(glue::translateMat4(glue::vec3(3.f, 2.f, 0.f)));

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
	

	std::string prtFragShaderFileName;
	if (nBands == 4) {
		prtFragShaderFileName = "PrtMono4Band.frag";
	}
	else if (nBands == 5) {
		prtFragShaderFileName = "PrtMono5Band.frag";
	}
	else {
		throw std::runtime_error("No shader implemented for " + std::to_string(nBands) + " SH bands.");
	}
	glue::ShaderProgram drawMeshProgram(std::vector<std::string> {
		glue::GLUE_SHADER_DIR + "PrtMono.vert",
		glue::GLUE_SHADER_DIR + prtFragShaderFileName
	});
	
	drawMeshProgram.setUniform("prtCoeffts", 0);
	drawMeshProgram.setUniform("albedo", albedo);
	drawMeshProgram.setUniform("lightingCoeffts", shCoeffts);
	
	glue::Mesh mesh;
	{
    	glue::ModelLoader loader(modelFilename);
    	mesh.fromModelLoader(loader);
	}
	mesh.shaderProgram(&drawMeshProgram);
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
					drawMeshProgram.setUniform("lightingCoeffts", shCoeffts);
				}
			}
		}
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		//Draw mesh.
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		prtCoefftArrTex.bindToImageUnit(0);
		mesh.render();

		//glDisable(GL_DEPTH_TEST);
		//glDisable(GL_CULL_FACE);
		shPlot.render();

		win.drawText(30, 30);
		glDepthMask(GL_TRUE);
		
		win.swapBuffers();
		
		SDL_Delay(30);
	}

	return 0;
}
