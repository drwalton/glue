#include "glue/ArrayTexture.hpp"
#include "glue/Directories.hpp"
#include "glue/GLWindow.hpp"
#include "glue/FullScreenQuad.hpp"
#include "glue/ShaderProgram.hpp"
#include <FL/Fl_Native_File_Chooser.H>

int main(int argc, char *argv[])
{
	glue::GLWindow win("PRT Textures", 640, 640);
	glue::ShaderProgram shader({
		glue::GLUE_SHADER_DIR + "FullScreenTex.vert",
		glue::GLUE_SHADER_DIR + "FullScreenArrayTexPosNegFlip.frag"
	});
	std::string filename;
	if (argc >= 2) {
		filename = argv[1];
	} else {
		Fl_Native_File_Chooser chooser(Fl_Native_File_Chooser::BROWSE_FILE);
		chooser.directory(glue::GLUE_MODEL_DIR.c_str());
		if (chooser.show() != 0) {
			return 1;
		}
		filename = chooser.filename();
	}

	bool running = true;
	SDL_Event event;

	win.updateText("glue");

	glue::ArrayTexture tex(filename);//, glue::ArrayTexture::PrtLoadMode::UCHAR);

	int currLevel = 0;
	shader.setUniform("tex", 0);
	shader.setUniform("layer", currLevel);
	win.updateText("Layer " + std::to_string(currLevel) + " of " + std::to_string(tex.nLayers()));
	tex.bindToImageUnit(0);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glClearColor(1, 0, 0, 1);

	while (running) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	

		glDisable(GL_BLEND);
		tex.bindToImageUnit(0);
		glue::FullScreenQuad::getInstance().render(shader);
		win.drawText(30, 30);

		while (SDL_PollEvent(&event)) {
			if (glue::GLWindow::eventIsQuit(event)) {
				running = false;
			}
			if (event.type == SDL_KEYDOWN) {
				if(event.key.keysym.sym == SDLK_SPACE) {
					currLevel = (currLevel + 1) % tex.nLayers();
					shader.setUniform("layer", currLevel);
					win.updateText("Layer " + std::to_string(currLevel) + " of " + std::to_string(tex.nLayers()));
				}
			}
		}
		win.swapBuffers();
		SDL_Delay(30);
	}

	return 0;
}
