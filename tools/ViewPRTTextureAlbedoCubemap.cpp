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

int main(int argc, char *argv[])	
{
	std::string modelFilename, textureFilename, cubemapDirectory, albedoTexFilename;
	if (argc == 5) {
		modelFilename = argv[1];
		textureFilename = argv[2];
		cubemapDirectory = argv[3];
		albedoTexFilename = argv[4];
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
		chooser.type(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
		chooser.title("Load Cubemap Image Folder");
		if (chooser.show() != 0) {
			return 1;
		}
		cubemapDirectory = chooser.filename();
		chooser.type(Fl_Native_File_Chooser::BROWSE_FILE);
		chooser.title("Load Albedo Texture File");
		if (chooser.show() != 0) {
			return 1;
		}
		albedoTexFilename = chooser.filename();
	}
	glue::GLWindow win("PRT Preview", 1280, 960);
	win.updateText("Test");
	glue::ArrayTexture prtCoefftArrTex(textureFilename);
	std::unique_ptr<glue::Texture> albedoTex;

	size_t nBands = glue::calcNumSHBands(prtCoefftArrTex.nLayers());
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

	{
		cv::Mat albedoTexImage = cv::imread(albedoTexFilename);
		cv::flip(albedoTexImage, albedoTexImage, 0);
		albedoTex.reset(new glue::Texture(GL_TEXTURE_2D, GL_RGB8, 
			albedoTexImage.cols, albedoTexImage.rows, 0, 
			GL_BGR, GL_UNSIGNED_BYTE, albedoTexImage.data));
	}

	std::string prtFragShaderFileName;
	if (nBands == 4) {
		prtFragShaderFileName = "PrtAlbedoTex4Band.frag";
	}
	else if (nBands == 5) {
		prtFragShaderFileName = "PrtAlbedoTex5Band.frag";
	}
	else {
		throw std::runtime_error("No shader implemented for " + std::to_string(nBands) + " SH bands.");
	}
	
	glue::ShaderProgram viewEnvMapProgram(std::vector<std::string> {
		glue::GLUE_SHADER_DIR + "FullScreenTex.vert",
		glue::GLUE_SHADER_DIR + "FullScreenTex_ViewEnvMap.frag"
	});
	glue::ShaderProgram drawMeshProgram(std::vector<std::string> {
		glue::GLUE_SHADER_DIR + "PrtMono.vert",
		glue::GLUE_SHADER_DIR + prtFragShaderFileName
	});
	
	viewEnvMapProgram.setUniform("cubemap", 0);
	drawMeshProgram.setUniform("prtCoeffts", 0);
	drawMeshProgram.setUniform("albedoTex", 1);
	drawMeshProgram.setUniform("lightingCoeffts", cubemapSH);
	
	glue::Mesh mesh;
	{
    	glue::ModelLoader loader(modelFilename);
    	mesh.fromModelLoader(loader);
	}
	mesh.shaderProgram(&drawMeshProgram);
	SDL_Event event;
	bool running = true;
	glClearColor(0, 1, 0, 1);
	
	while(running) {
		while(SDL_PollEvent(&event)) {
			viewer.processEvent(event);
			if(glue::GLWindow::eventIsQuit(event)) {
				running = false;
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
		
		//Draw mesh.
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		prtCoefftArrTex.bindToImageUnit(0);
		albedoTex->bindToImageUnit(1);
		mesh.render();

		cubemapSHPlot.render();

		win.drawText(30, 30);
		glDepthMask(GL_TRUE);
		
		win.swapBuffers();
		
		SDL_Delay(30);
	}

	return 0;
}
