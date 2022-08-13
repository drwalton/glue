#include "glue/GLWindow.hpp"
#include "glue/PrtMeshColorShadowed.hpp"
#include "glue/ModelLoader.hpp"
#include "glue/RotateViewer.hpp"
#include "glue/FullScreenQuad.hpp"
#include "glue/CubemapRenderer.hpp"
#include "glue/CubemapProjector.hpp"
#include "glue/Directories.hpp"
#include <Eigen/Dense>

using namespace glue;

int main(int argc, char *argv[])
{
	float lodLevel = 6.f, refractIdx = 0.8f;
	GLWindow win("Rendering Test", 1280, 960);
	glViewport(0, 0, 1280, 960);
	RotateViewer viewer(&win);
	win.makeCurrent();

	CubemapRenderer cubemap(512);
	cubemap.loadExampleCubemap();

	PrtMeshColorShadowed mesh(
		glue::GLUE_MODEL_DIR + "empty.obj",
		glue::GLUE_MODEL_DIR + "sphereJustPlaneSpecularDifferential.obj.s_500_b_4_w_1024_lb_2.prtc",
		glue::GLUE_MODEL_DIR + "sphereJustPlane.obj");
	SHProjection3 lighting;
	{
		CubemapProjector projector(mesh.nBands(), 512);

		std::array<cv::Mat, 6> faceMats;
		for (size_t f = 0; f < 6; ++f) {
			faceMats[f] = cv::Mat(512, 512, CV_8UC4);
			cubemap.getImage(f, faceMats[f].data);
		}
		projector.projectCubemap(faceMats);
		lighting = projector.proj();
	}

	for(auto &l : lighting) {
		l *= 20.f;
	}
	mesh.lightingCoeffts(lighting);

	ShaderProgram quadProgram(std::vector<std::string> {
		glue::GLUE_SHADER_DIR + "FullScreenTex.vert",
		glue::GLUE_SHADER_DIR + "FullScreenTex_ViewEnvMap.frag"
	});

	bool running = true;
	SDL_Event event;
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, 1280, 960);
	glClearColor(.4f, .4f, .4f, 1.f);
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap.texture());
	quadProgram.setUniform("cubemap", 0);
	mat4 identity4 = mat4::Identity();
	mat3 identity3 = mat3::Identity();

	while (running) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glDepthMask(GL_FALSE);
		glDisable(GL_DEPTH_TEST);
		mat3 invRot = viewer.rotation().inverse();
		quadProgram.setUniform("invRotMat", invRot);
		FullScreenQuad::getInstance().render(quadProgram);


		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		mesh.render();

		win.swapBuffers();
		SDL_PollEvent(&event);
		viewer.processEvent(event);
		if (event.type == SDL_QUIT) {
			running = false;
		}
		if (event.type == SDL_WINDOWEVENT) {
			if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
				running = false;
			}
		}
		if (event.type == SDL_KEYDOWN) {
			if (event.key.keysym.sym == SDLK_l) {
				if (event.key.keysym.mod & SDL_Keymod::KMOD_LSHIFT) {
					lodLevel -= 0.5f;
				}
				else {
					lodLevel += 0.5f;
				}
			}
			else if (event.key.keysym.sym == SDLK_r) {
				if (event.key.keysym.mod & SDL_Keymod::KMOD_LSHIFT) {
					refractIdx += 0.05f;
				}
				else {
					refractIdx -= 0.05f;
				}
			}
		}

		SDL_Delay(30);
	}

	return 0;
}
