#include "glue/GLWindow.hpp"
#include "glue/WaveMesh.hpp"
#include "glue/RotateViewer.hpp"
#include <Eigen/Dense>

using namespace glue;

int main(int argc, char *argv[])
{
	GLWindow win("Rendering Test", 1280, 960, 30, 30, true);
	RotateViewer viewer(&win);
	win.makeCurrent();
	
	WaveMesh mesh;

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
		mesh.render();
		mesh.update();
		win.swapBuffers();

		while(SDL_PollEvent(&event)) {
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
