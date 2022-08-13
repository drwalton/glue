#include "glue/GLWindow.hpp"

int main(int argc, char *argv[])
{
	glue::GLWindow win("Test Window", 640, 480, 0, 0, true);
	
	bool running = true;
	SDL_Event event;
	
	float color = 0.f;
	
	win.updateText("glue");
	
	while (running) {
		glClearColor(
    		0.5f*sinf(color) + 1.f,
    		0.5f*sinf(2.f*color) + 1.f,
    		0.5f*sinf(3.f*color) + 1.f,
			1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		win.drawText(30, 30);
		
		color += 0.01f;
		if(color > 100.f) {
			color -= 100.f;
		}
		
		SDL_PollEvent(&event);
		if(glue::GLWindow::eventIsQuit(event)) {
			running = false;
		} else if (event.type == SDL_KEYDOWN) {
			if (event.key.keysym.sym == SDLK_f) {
				win.fullscreen(!win.fullscreen());
			}
		}
		win.swapBuffers();
		SDL_Delay(30);
	}

	return 0;
}
