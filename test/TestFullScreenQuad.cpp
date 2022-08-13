#include "glue/Directories.hpp"
#include "glue/GLWindow.hpp"
#include "glue/FullScreenQuad.hpp"
#include "glue/ShaderProgram.hpp"

int main(int argc, char *argv[])
{
	glue::GLWindow win("Test Window", 640, 480);
	glue::ShaderProgram shader({
		glue::GLUE_SHADER_DIR + "TestFullScreenQuad.vert",
		glue::GLUE_SHADER_DIR + "TestFullScreenQuad.frag"
	});

	bool running = true;
	SDL_Event event;

	win.updateText("glue");

	while (running) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glue::FullScreenQuad::getInstance().render(shader);

		SDL_PollEvent(&event);
		if(glue::GLWindow::eventIsQuit(event)) {
			running = false;
		}
		win.swapBuffers();
		SDL_Delay(30);
	}

	return 0;
}
