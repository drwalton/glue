#include "glue/ArrayTexture.hpp"
#include "glue/Directories.hpp"
#include "glue/GLWindow.hpp"
#include "glue/FullScreenQuad.hpp"
#include "glue/ShaderProgram.hpp"
#include <SDL.h>

const size_t texWidth = 512, texHeight = 512, texLevels = 16;
const GLubyte layerColors[texLevels][3] = {
	{128, 0, 128},
	{0, 128, 0},
	{0, 0, 128},
	{0, 128, 128},

	{128, 0, 128},
	{0, 128, 0},
	{0, 0, 128},
	{0, 128, 128},

	{128, 0, 128},
	{0, 128, 0},
	{0, 0, 128},
	{0, 128, 128},

	{128, 0, 128},
	{0, 128, 0},
	{0, 0, 128},
	{0, 128, 128}
};

const float layerColorsFloat[texLevels] = {
	1,0,0.5,0.75,
	1,0,0.5,0.75,
	1,0,0.5,0.75,
	1,0,0.5,0.75
};

int main(int argc, char *argv[])
{
	glue::GLWindow win("Test Window", 640, 480);
	glue::ShaderProgram shader({
		glue::GLUE_SHADER_DIR + "FullScreenTex.vert",
		glue::GLUE_SHADER_DIR + "FullScreenArrayTexFlip.frag"
	});

	bool running = true;
	SDL_Event event;

	win.updateText("glue");

	glue::ArrayTexture tex(
		texWidth, texHeight,
		texLevels,
		GL_RGB, GL_UNSIGNED_BYTE, GL_RGB32F);
	GLubyte *im = new GLubyte[texWidth*texHeight * 3];
	for (size_t level = 0; level < texLevels; ++level) {
		for (size_t i = 0; i < texWidth*texHeight; ++i) {
			im[i * 3 + 0] = layerColors[level][0];
			im[i * 3 + 1] = layerColors[level][1];
			im[i * 3 + 2] = layerColors[level][2];
		}
		tex.update(level, im);
	}
	delete[](im);

	glue::ArrayTexture tex2(
		texWidth, texHeight,
		texLevels,
		GL_RED, GL_FLOAT, GL_R8);
	float *imf = new float[texWidth*texHeight];
	for (size_t level = 0; level < texLevels; ++level) {
		for (size_t i = 0; i < texWidth*texHeight; ++i) {
			imf[i] = layerColorsFloat[level];
		}
		tex2.update(level, imf);
	}
	delete[](imf);

	int currLevel = 0;
	shader.setUniform("tex", 0);
	shader.setUniform("layer", currLevel);
	win.updateText("Layer " + std::to_string(currLevel) + " of " + std::to_string(texLevels));
	tex.bindToImageUnit(0);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glClearColor(1, 0, 0, 1);

	while (running) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	

		glDisable(GL_BLEND);
		tex2.bindToImageUnit(0);
		glue::FullScreenQuad::getInstance().render(shader);
		win.drawText(30, 30);

		while (SDL_PollEvent(&event)) {
			if (glue::GLWindow::eventIsQuit(event)) {
				running = false;
			}
			if (event.type == SDL_KEYDOWN) {
				if(event.key.keysym.sym == SDLK_SPACE) {
					currLevel = (currLevel + 1) % texLevels;
					shader.setUniform("layer", currLevel);
					win.updateText("Layer " + std::to_string(currLevel) + " of " + std::to_string(texLevels));
				}
			}
		}
		win.swapBuffers();
		glFlush();
		SDL_Delay(30);
	}

	return 0;
}
