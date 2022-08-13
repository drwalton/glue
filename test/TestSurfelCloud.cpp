#include "glue/Directories.hpp"
#include "glue/GLBuffer.hpp"
#include "glue/GLWindow.hpp"
#include "glue/ShaderProgram.hpp"
#include "glue/SurfelCloud.hpp"

int main(int argc, char *argv[])
{
	std::vector<glue::vec3> verts {
		glue::vec3(0,0,0), glue::vec3(0,1,0)
	};
	std::vector<glue::vec3> norms {
		glue::vec3(1,0,0), glue::vec3(1,0,0)
	};
	std::vector<float> radii {
		0.25f, 0.25f
	};

	glue::GLWindow win("Test Window", 640, 480);
	glue::ShaderProgram shader({
		glue::GLUE_SHADER_DIR + "TestSurfelCloud.vert",
		glue::GLUE_SHADER_DIR + "TestSurfelCloud.geom",
		glue::GLUE_SHADER_DIR + "TestSurfelCloud.frag"
	});
	glue::CameraBlockBuffer cameraBlock;
	cameraBlock.bindRange(0);
	glue::SurfelCloud surfelCloud(verts, norms, radii);
	surfelCloud.shaderProgram(&shader);

	bool running = true;
	SDL_Event event;

	win.updateText("glue");

	while (running) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		surfelCloud.render();

		SDL_PollEvent(&event);
		if(glue::GLWindow::eventIsQuit(event)) {
			running = false;
		}
		win.swapBuffers();
		SDL_Delay(30);
	}

	return 0;
}
