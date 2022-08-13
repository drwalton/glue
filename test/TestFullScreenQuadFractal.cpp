#include "glue/Directories.hpp"
#include "glue/GLWindow.hpp"
#include "glue/FullScreenQuad.hpp"
#include "glue/ShaderProgram.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int winWidth = 640, winHeight = 480;
double scale = 4.f;
double scaleSpeed = 0.05f;
double cSpeed = 0.01;
int maxIters = 20;
int mandlebrotPower = 2;
double aspect = double(winWidth) / double(winHeight);
glue::dvec2 offset(0.f, 0.f);
glue::dvec2 c(0.1f, 0.65f);
glue::dvec2 cycleC;
glue::dvec2 offsetSpeed(1.f / float(winWidth), 1.f / float(winHeight));

double cycleTheta = 0.f;
double cycleSpeed = 0.01f;
double cycleR = 1.f;

enum class Mode {
	MULTIBROT,
	JULIA_SET,
	JULIA_CYCLE
};
std::vector<Mode> modes {
	Mode::MULTIBROT,
	Mode::JULIA_SET,
	Mode::JULIA_CYCLE
};
size_t modeIndex = 0;

std::unique_ptr<glue::ShaderProgram> multibrotShader, juliaShader;

void setTitle(glue::GLWindow *win)
{
	if(modes[modeIndex] == Mode::MULTIBROT) {
    	win->setTitle("Multibrot Set: Power " + std::to_string(mandlebrotPower) + " " + std::to_string(4.0/scale) + "x");
	} else if (modes[modeIndex] == Mode::JULIA_SET) {
    	win->setTitle("Julia Set: Power " + std::to_string(mandlebrotPower) + " " + std::to_string(4.0/scale) + "x: c = " + std::to_string(c.x()) + " + " + std::to_string(c.y()) + "i");
	} else if (modes[modeIndex] == Mode::JULIA_CYCLE){
    	win->setTitle("Julia Set: Power " + std::to_string(mandlebrotPower) + " " + std::to_string(4.0/scale) + "x: c = " + std::to_string(cycleC.x()) + " + " + std::to_string(cycleC.y()) + "i");
	}
}
void render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (modes[modeIndex] == Mode::MULTIBROT) {
		glue::FullScreenQuad::getInstance().render(*multibrotShader);
	} else if (modes[modeIndex] == Mode::JULIA_SET) {
		glue::FullScreenQuad::getInstance().render(*juliaShader);
	} else if (modes[modeIndex] == Mode::JULIA_CYCLE) {
		glue::FullScreenQuad::getInstance().render(*juliaShader);
	}
}

int main(int argc, char *argv[])
{

	glue::GLWindow win("Mandlebrot Set", winWidth, winHeight, 30, 30, true);
	multibrotShader.reset(new glue::ShaderProgram({
		glue::GLUE_SHADER_DIR + "TestFullScreenQuad.vert",
		glue::GLUE_SHADER_DIR + "TestFullScreenQuadFractal.frag"
	}));
	juliaShader.reset(new glue::ShaderProgram({
		glue::GLUE_SHADER_DIR + "TestFullScreenQuad.vert",
		glue::GLUE_SHADER_DIR + "TestFullScreenQuadJulia.frag"
	}));

	bool running = true;
	SDL_Event event;

	win.updateText("glue");
	
	multibrotShader->setUniform("offset", offset);
	multibrotShader->setUniform("scale", scale);
	multibrotShader->setUniform("maxIters", maxIters);
	multibrotShader->setUniform("aspect", aspect);
	multibrotShader->setUniform("power", mandlebrotPower);
	juliaShader->setUniform("offset", offset);
	juliaShader->setUniform("scale", scale);
	juliaShader->setUniform("maxIters", maxIters);
	juliaShader->setUniform("aspect", aspect);
	juliaShader->setUniform("c", c);
	juliaShader->setUniform("power", mandlebrotPower);
	setTitle(&win);
	render();

	while (running) {
		std::this_thread::sleep_for(std::chrono::milliseconds(30));
		
		if (modes[modeIndex] == Mode::JULIA_CYCLE) {
			cycleTheta += cycleSpeed;
			cycleTheta = fmodf(cycleTheta, M_PI * 2.0);
			cycleC = glue::dvec2(cycleR * sin(cycleTheta), cycleR * cos(cycleTheta));
			juliaShader->setUniform("c", cycleC);
			setTitle(&win);
			render();
		}

		while(SDL_PollEvent(&event)) {
			glue::ShaderProgram *currShader;
			if(modes[modeIndex] == Mode::MULTIBROT) {
				currShader = multibrotShader.get();
			} else if (modes[modeIndex] == Mode::JULIA_SET ||
				modes[modeIndex] == Mode::JULIA_CYCLE) {
				currShader = juliaShader.get();
			}
    		if(glue::GLWindow::eventIsQuit(event)) {
    			running = false;
    		}
        	if (event.type == SDL_MOUSEMOTION || event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_WINDOWEVENT) {
        		if (event.window.windowID == win.id()) {
        			static int lastx = event.motion.x;
        			static int lasty = event.motion.y;
        			if (event.type == SDL_MOUSEBUTTONDOWN) {
        				if (event.motion.state & SDL_BUTTON(1)) {
        					lastx = event.motion.x;
        					lasty = event.motion.y;
        				}
        			}
        			if (event.type == SDL_MOUSEMOTION) {
        				if (event.motion.state & SDL_BUTTON(1)) {
        					offset.x() -= (event.motion.x - lastx) * offsetSpeed.x() * scale;
        					offset.y() += (event.motion.y - lasty) * offsetSpeed.y() * scale;
        					lastx = event.motion.x;
        					lasty = event.motion.y;
							currShader->setUniform("offset", offset);
							render();
        				}
        			}
        		}
        	}
        	if (event.type == SDL_WINDOWEVENT) {
				if(event.window.event == SDL_WINDOWEVENT_RESIZED) {
					winWidth = event.window.data1;
					winHeight = event.window.data2;
					aspect = float(winWidth) / float(winHeight);
					currShader->setUniform("aspect", aspect);
					offsetSpeed = glue::dvec2(1.f / double(winWidth), 1.f / double(winHeight));
					glViewport(0, 0, winWidth, winHeight);
					render();
				}
			}
        	if (event.type == SDL_MOUSEWHEEL) {
				scale += event.wheel.y * scaleSpeed * scale;
				if(scale < 0.f) scale = 0.f;
				currShader->setUniform("scale", scale);
				setTitle(&win);
				render();
        	}
			if(event.type == SDL_KEYDOWN) {
				if(event.key.keysym.sym == SDLK_EQUALS) {
					++maxIters;
					currShader->setUniform("maxIters", maxIters);
					render();
				}
				if(event.key.keysym.sym == SDLK_MINUS) {
					--maxIters;
					currShader->setUniform("maxIters", maxIters);
					render();
				}
				if(event.key.keysym.sym == SDLK_SPACE) {
					if(modes[modeIndex] == Mode::JULIA_CYCLE) {
						juliaShader->setUniform("c", c);
					}
					++modeIndex;
					modeIndex %= modes.size();
					setTitle(&win);
					render();
				}
				if(modes[modeIndex] == Mode::JULIA_SET) {
					if(event.key.keysym.sym == SDLK_RIGHT ||
						event.key.keysym.sym == SDLK_UP ||
						event.key.keysym.sym == SDLK_LEFT ||
						event.key.keysym.sym == SDLK_DOWN) {
						
        				if(event.key.keysym.sym == SDLK_RIGHT) {
        					c.x() += cSpeed;
        				} else if (event.key.keysym.sym == SDLK_LEFT) {
        					c.x() -= cSpeed;
        				} else if (event.key.keysym.sym == SDLK_DOWN) {
        					c.y() -= cSpeed;
        				} else if (event.key.keysym.sym == SDLK_UP) {
        					c.y() += cSpeed;
        				}
    					juliaShader->setUniform("c", c);
    					render();
    					setTitle(&win);
    				}
				}
				if(modes[modeIndex] == Mode::JULIA_CYCLE) {
					if(event.key.keysym.sym == SDLK_RIGHT ||
						event.key.keysym.sym == SDLK_UP ||
						event.key.keysym.sym == SDLK_LEFT ||
						event.key.keysym.sym == SDLK_DOWN) {
						
        				if(event.key.keysym.sym == SDLK_RIGHT) {
							cycleSpeed += 0.0001f;
        				} else if (event.key.keysym.sym == SDLK_LEFT) {
							cycleSpeed -= 0.0001f;
        				} else if (event.key.keysym.sym == SDLK_DOWN) {
							cycleR += 0.01f;
        				} else if (event.key.keysym.sym == SDLK_UP) {
							if(cycleR > 0.01) {
								cycleR -= 0.01f;
							}
        				}
    				}
				}
				if(event.key.keysym.sym == SDLK_9) {
					if(mandlebrotPower > 1) {
						mandlebrotPower--;
						currShader->setUniform("power", mandlebrotPower);
						setTitle(&win);
						render();
					}
				}
				if(event.key.keysym.sym == SDLK_0) {
					mandlebrotPower++;
					currShader->setUniform("power", mandlebrotPower);
					setTitle(&win);
					render();
				}
			}
		}
		
		win.swapBuffers();
		SDL_Delay(30);
	}

	return 0;
}
