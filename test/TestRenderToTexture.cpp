#include "glue/GLWindow.hpp"
#include "glue/Mesh.hpp"
#include "glue/ModelLoader.hpp"
#include "glue/RotateViewer.hpp"
#include "glue/RenderToTexture.hpp"
#include "glue/FullScreenQuad.hpp"
#include "glue/CubemapRenderer.hpp"
#include "glue/Directories.hpp"
#include <Eigen/Dense>

using namespace glue;

int main(int argc, char *argv[])
{
	GLWindow win("Rendering Test", 1280, 960, 30, 30, true);

	RenderToTexture tex(GL_RGB, 1280, 960, 0, GL_RGB, GL_UNSIGNED_BYTE);

	RotateViewer viewer(&win);
	win.makeCurrent();
	
	ShaderProgram showTexShader(std::vector<std::string> {
		GLUE_SHADER_DIR + "FullScreenTex.vert",
		GLUE_SHADER_DIR + "FullScreenTex.frag"
	});
	showTexShader.setUniform("tex", 0);
	ShaderProgram meshShader(std::vector<std::string> {
		GLUE_SHADER_DIR + "GoochGreen.vert",
		GLUE_SHADER_DIR + "GoochGreen.frag"
	});

	Mesh mesh;
	{
		ModelLoader modelLoader;
		modelLoader.loadFile(GLUE_MODEL_DIR + "bunny.obj");
		mesh.fromModelLoader(modelLoader);
	}
	mesh.shaderProgram(&meshShader);
	{
		mat4 identity4 = mat4::Identity();
		mat3 identity3 = mat3::Identity();
		mesh.shaderProgram()->setUniform("modelToWorld", identity4);
		mesh.shaderProgram()->setUniform("normToWorld", identity3);
	}

	bool running = true;
	SDL_Event event;
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glClearColor(.4f, .4f, .4f, 1.f);

	while (running) {
		tex.setAsRenderTarget();
		glViewport(0, 0, 1280, 960);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		mesh.render();
		tex.unsetAsRenderTarget();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, 1280, 960);
		glDisable(GL_DEPTH_TEST);
		tex.texture().bindToImageUnit(0);
		showTexShader.use();
		FullScreenQuad::getInstance().render(showTexShader);
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
