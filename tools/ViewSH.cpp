#include "glue/GLWindow.hpp"
#include "glue/RotateViewer.hpp"
#include "glue/FullScreenQuad.hpp"
#include "glue/Directories.hpp"
#include "glue/Matrices.hpp"
#include "glue/SphericalPlot.hpp"
#include "glue/SphericalHarmonics.hpp"
#include <Eigen/Dense>

using namespace glue;

int main(int argc, char *argv[])
{
	GLWindow win("Rendering Test", 1280, 960, 30, 30, true);
	RotateViewer viewer(&win);
	win.makeCurrent();

	size_t nBands = 7;
	float spacing = 1.1f;
	std::vector<SphericalPlot*> sphericalPlots;

	for (int band = 0; band < nBands; ++band) {
		for (int m = -band; m <= band; ++m) {
			sphericalPlots.push_back(new glue::SphericalPlot(
				[&band, &m](float theta, float phi) {
				glue::vec3 dir = glue::sphericalAngleToDir(theta, phi);
				//return glue::realSH(band, m, theta, phi);
				return glue::realSH(band, m, dir);
			}, 30));
			sphericalPlots.back()->modelToWorld(glue::translateMat4(glue::vec3(m*spacing, band*spacing, 0.f)));
		}
	}

	bool running = true;
	SDL_Event event;
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glClearColor(.4f, .4f, .4f, 1.f);

	while (running) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);

		for (auto &p : sphericalPlots) {
			p->render();
		}
		win.swapBuffers();

		while (SDL_PollEvent(&event)) {
			viewer.processEvent(event);
			if (event.type == SDL_QUIT) {
				running = false;
			}
			if (event.type == SDL_WINDOWEVENT) {
				if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
					running = false;
				}
			}
		}

		viewer.update();
		SDL_Delay(30);
	}

	return 0;
}
