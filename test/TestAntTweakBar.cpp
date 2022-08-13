#include "glue/GLWindow.hpp"
#include "glue/AntTweakBar.hpp"
#include <sstream>

void onButtonPress(void *data)
{
	static int timesPressed = 0;
	++timesPressed;
	glue::GLWindow *win = reinterpret_cast<glue::GLWindow*>(data);
	win->updateText("Button pressed " + std::to_string(timesPressed) + " times");
}

int main(int argc, char *argv[])
{
	glue::GLWindow win("Test Window", 640, 480);
	glue::AntTweakBar bar(&win);

	int intrw = 101;
	const int intro = 101;
	float floatrw = 0.1f;
	const float floatro = 0.1f;
	bool boolrw = false;
	glue::vec3 colorrw(0.5, 0.5, 0.5);
	const glue::vec3 colorro(0.5, 0.5, 0.5);
	glue::vec3 dirrw(1.0, 0.0, 0.0);
	const glue::vec3 dirro(0.0, 1.0, 0.0);
	std::string stringro = "0 Events Processed";
	bar.addVarRW("intrw", &intrw, "intrw", 0, 1000);
	bar.addVarRO("intro", &intro, "intro");
	bar.addVarRW("floatrw", &floatrw, "floatrw", 0.f, 1.f, 0.01f);
	bar.addVarRO("floatro", &floatro, "floatro");
	bar.addVarRW("boolrw", &boolrw, "boolrw");
	bar.addColorRW("colorrw", &colorrw, "colorrw");
	bar.addColorRO("colorro", &colorro, "colorro");
	bar.addDirRW("dirrw", &dirrw, "dirrw");
	bar.addDirRO("dirro", &dirro, "dirro");
	bar.addVarRO("stringro", &stringro, "stringro");
	bar.addButton("button", onButtonPress, &win, "Press Me");

	bool running = true;
	SDL_Event event;

	win.updateText("glue");

	while (running) {
		glClearColor(colorrw.x(), colorrw.y(), colorrw.z(), 0.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	

		glViewport(0, 0, 640, 480);
		bar.draw();

		win.drawText(300, 30);

		while (SDL_PollEvent(&event)) {
			if (glue::GLWindow::eventIsQuit(event)) {
				running = false;
			}
			bar.processEvent(event);
			static int eventsProcessed = 0;
			++eventsProcessed;
			stringro = std::to_string(eventsProcessed) + " Events Processed";
		}
		win.swapBuffers();
		glFlush();
		SDL_Delay(30);
	}

	return 0;
}
