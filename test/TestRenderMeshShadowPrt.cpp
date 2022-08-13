#include "glue/GLWindow.hpp"
#include "glue/Mesh.hpp"
#include "glue/ModelLoader.hpp"
#include "glue/RotateViewer.hpp"
#include "glue/FullScreenQuad.hpp"
#include "glue/CubemapRenderer.hpp"
#include "glue/Directories.hpp"
#include <Eigen/Dense>

using namespace glue;

int main(int argc, char *argv[])
{
	float lodLevel = 6.f, refractIdx = 0.8f;
	GLWindow win("Rendering Test", 1280, 960);
	RotateViewer viewer(&win);
	win.makeCurrent();
	ShaderProgram reflectShaderProgram(std::vector<std::string> {
		"ReflectCubemap.vert",
			"ReflectCubemap.frag"
	});
	ShaderProgram reflectRoughShaderProgram(std::vector<std::string> {
		"ReflectCubemap.vert",
			"ReflectCubemapRough.frag"
	});
	ShaderProgram reflectRoughColorShaderProgram(std::vector<std::string> {
		"ReflectCubemap.vert",
			"ReflectCubemapRoughColor.frag"
	});
	ShaderProgram refractShaderProgram(std::vector<std::string> {
		"RefractCubemap.vert",
			"RefractCubemap.frag"
	});
	ShaderProgram refractRoughShaderProgram(std::vector<std::string> {
		"RefractCubemap.vert",
			"RefractCubemapRough.frag"
	});
	ShaderProgram reflectMultiShaderProgram(std::vector<std::string> {
		"ReflectCubemap.vert",
			"ReflectCubemapMultisample.frag"
	});
	//ShaderProgram reflectAnisoShaderProgram(std::vector<std::string> { 
	//	"ReflectCubemap.vert", 
	//	"ReflectCubemapAniso.frag" 
	//});
	ShaderProgram refractRoughColorShaderProgram(std::vector<std::string> {
		"RefractCubemap.vert",
			"RefractCubemapRoughColor.frag"
	});
	ShaderProgram quadProgram(std::vector<std::string> {
		"FullScreenTex.vert",
			"FullScreenTex_ViewEnvMap.frag"
	});

	std::vector<ShaderProgram*> shaderList{
		&reflectShaderProgram,
		&reflectRoughShaderProgram,
		&reflectRoughColorShaderProgram,
		&reflectMultiShaderProgram,
		//&reflectAnisoShaderProgram,
		&refractShaderProgram,
		&refractRoughShaderProgram,
		&refractRoughColorShaderProgram
	};
	std::vector<std::string> shaderNames{
		"Mirror Reflection",
		"Rough Reflection",
		"Rough Reflection (Color)",
		"Rough Reflection (Multisample)",
		//"Rough Reflection (Aniso)",
		"Perfect Refraction",
		"Rough Refraction",
		"Rough Refraction (Color)"
	};
	size_t shaderIdx = 0;

	CubemapRenderer cubemap(512);
	cubemap.loadExampleCubemap();

	Mesh mesh;
	{
		ModelLoader modelLoader;
		modelLoader.loadFile(GLUE_MODEL_DIR + "monkey.obj");
		mesh.fromModelLoader(modelLoader);
	}
	mesh.shaderProgram(shaderList[shaderIdx]);
	std::stringstream winText;
	winText << shaderNames[shaderIdx] << ", Roughness: " << lodLevel << ", RI: " << refractIdx;
	win.updateText(winText.str());

	bool running = true;
	SDL_Event event;
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, 640, 480);
	glClearColor(.4f, .4f, .4f, 1.f);
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap.texture());
	quadProgram.setUniform("cubemap", 0);
	mat4 identity4 = mat4::Identity();
	mat3 identity3 = mat3::Identity();

	while (running) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glDepthMask(GL_FALSE);
		glDisable(GL_DEPTH_TEST);
		mat3 invRot = viewer.rotation().inverse();
		quadProgram.setUniform("invRotMat", invRot);
		FullScreenQuad::getInstance().render(quadProgram);


		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		mesh.shaderProgram()->setUniform("modelToWorld", identity4);
		mesh.shaderProgram()->setUniform("normToWorld", identity3);
		mesh.shaderProgram()->setUniform("cubemap", 0);
		mesh.shaderProgram()->setUniform("lodLevel", lodLevel);
		mesh.shaderProgram()->setUniform("eta", refractIdx);
		mesh.shaderProgram()->setUniform("camPosModelSpace", viewer.position());
		mesh.render();

		glDisable(GL_DEPTH_TEST);
		win.drawText(30, 30);
		win.swapBuffers();

		SDL_PollEvent(&event);
		viewer.processEvent(event);
		if (event.type == SDL_QUIT) {
			running = false;
		}
		if (event.type == SDL_WINDOWEVENT) {
			if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
				running = false;
			}
		}
		if (event.type == SDL_KEYDOWN) {
			if (event.key.keysym.sym == SDLK_l) {
				if (event.key.keysym.mod & SDL_Keymod::KMOD_LSHIFT) {
					lodLevel -= 0.5f;
				}
				else {
					lodLevel += 0.5f;
				}
			}
			else if (event.key.keysym.sym == SDLK_r) {
				if (event.key.keysym.mod & SDL_Keymod::KMOD_LSHIFT) {
					refractIdx += 0.05f;
				}
				else {
					refractIdx -= 0.05f;
				}
			}
			else if (event.key.keysym.sym == SDLK_SPACE) {
				shaderIdx += 1; shaderIdx %= shaderList.size();
				mesh.shaderProgram(shaderList[shaderIdx]);
			}
			std::stringstream winText;
			winText << shaderNames[shaderIdx] << ", Roughness: " << lodLevel << ", RI: " << refractIdx;
			win.updateText(winText.str());
		}

		SDL_Delay(30);
	}

	return 0;
}
