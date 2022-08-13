#include "glue/Directories.hpp"
#include "glue/FullScreenQuad.hpp"
#include "glue/GLWindow.hpp"
#include "glue/ShaderProgram.hpp"
#include "glue/Texture.hpp"
#include <iostream>

size_t winWidth = 640;
size_t winHeight = 480;

int main(int argc, char *argv[])
{
	glue::GLWindow win("compute shader demo", winWidth, winHeight);

	glue::ShaderProgram showImageShader({
		glue::GLUE_SHADER_DIR + "FullScreenTex.vert",
		glue::GLUE_SHADER_DIR + "FullScreenTexFlip.frag",
	});
	glue::ShaderProgram updateImageShader({
		glue::GLUE_SHADER_DIR + "TestComputeShaderImage.comp"
	});
	showImageShader.setUniform("tex", 0);

	glue::Texture tex(GL_TEXTURE_2D, GL_RGBA8, winWidth, winHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE);
	tex.bindToImageUnit(0);

	bool running = true;
	SDL_Event event;

	while (running) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		updateImageShader.use();
		static float time = 0.f;
		updateImageShader.setUniform("time", time);
		time += 0.01f;
		tex.bindToImageUnit(0);
		glBindImageTexture(0, tex.tex(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8UI);
		glDispatchCompute(winWidth, winHeight, 1);

		tex.bindToImageUnit(0);
		glue::FullScreenQuad::getInstance().render(showImageShader);

		win.swapBuffers();

		while(SDL_PollEvent(&event)) {

			if (event.type == SDL_QUIT) {
				running = false;
			}
			if (event.type == SDL_WINDOWEVENT) {
				if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
					running = false;
				}
			}
		}
		SDL_Delay(30);
	}
	
	return 0;
}
