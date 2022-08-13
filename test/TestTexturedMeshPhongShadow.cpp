#include "glue/GLWindow.hpp"
#include "glue/TexturedPhongMesh.hpp"
#include "glue/ModelLoader.hpp"
#include "glue/RotateViewer.hpp"
#include "glue/FullScreenQuad.hpp"
#include "glue/CubemapRenderer.hpp"
#include "glue/Directories.hpp"
#include <Eigen/Dense>

using namespace glue;

int main(int argc, char *argv[])
{
	GLWindow win("Rendering Test", 1280, 960, 30, 30, true);
	RotateViewer viewer(&win);
	win.makeCurrent();

	TexturedPhongMesh mesh(
		GLUE_MODEL_DIR + "laptop.jpg",
		GLUE_MODEL_DIR + "laptop.obj"
	);
	mesh.drawShadow(true);
	Plane plane;
	plane.N = vec3(0.f, 1.f, 0.f);
	plane.d = 0.f;
	mesh.shadowPlane(plane);
	mesh.lightWorldPos(vec3(0.f, 5.f, 0.f));
	
	vec3 lightCenter(0.f, 5.f, 0.f);
	float lightRadius = 1.f;
	float lightTheta = 0.f;

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
		win.swapBuffers();
		
		lightTheta += 0.01f;
		mesh.lightWorldPos(lightCenter + lightRadius*vec3(cosf(lightTheta), 0.f, sinf(lightTheta)));

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
