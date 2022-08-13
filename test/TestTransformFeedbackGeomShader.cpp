#include "glue/Directories.hpp"
#include "glue/GLWindow.hpp"
#include "glue/GLBuffer.hpp"
#include "glue/ShaderProgram.hpp"
#include "glue/VertexArrayObject.hpp"
#include <Eigen/Dense>
#include <iostream>

using namespace glue;

int main(int argc, char *argv[])
{
	GLWindow win("Rendering Test", 200, 200);
	win.makeCurrent();

	std::vector<float> inputVector = { 1.f, 2.f, 3.f, 4.f, 5.f, 6.f };
	std::vector<float> outputVector = { -1.f, -1.f, -1.f, -1.f, -1.f, -1.f};

	glue::VertexBuffer inputBuffer(inputVector);
	glue::VertexBuffer outputBuffer(outputVector);
	std::vector<std::string> tfVaryings = { "outVal" };
	glue::ShaderProgram testProgram(
		{GLUE_SHADER_DIR + "TestTransformFeedbackGeomShader.vert",
		GLUE_SHADER_DIR + "TestTransformFeedbackGeomShader.geom"}, 
		&tfVaryings, ShaderProgram::TFMode::INTERLEAVED_ATTRIBS);

	GLuint query;
	glGenQueries(1, &query);
	VertexArrayObject vao;
	vao.bind();
	inputBuffer.bind();
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, 0);
	inputBuffer.unbind();
	vao.unbind();
	
	bool running = true;
	SDL_Event e;
	testProgram.use();
	vao.bind();
	glEnable(GL_RASTERIZER_DISCARD);

	//testProgram.bindTransformFeedbackBuffer(0, outputBuffer.get());
	glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, outputBuffer.get(), 1 * sizeof(float), 5 * sizeof(float));
	glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, query);
	glBeginTransformFeedback(GL_POINTS);
	glDrawArrays(GL_POINTS, 0, 6);
	glEndTransformFeedback();
	glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
	glDisable(GL_RASTERIZER_DISCARD);
	vao.unbind();
	testProgram.unuse();

	glFlush();

	outputBuffer.getData(outputVector.data());
	GLuint nPrimitivesWritten;
	glGetQueryObjectuiv(query, GL_QUERY_RESULT, &nPrimitivesWritten);
	
	std::cout << "Input\n";
	for (size_t i = 0; i < inputVector.size(); ++i) {
		std::cout << inputVector[i] << std::endl;
	}

	std::cout << "Output\n";
	for (size_t i = 0; i < outputVector.size(); ++i) {
		std::cout << outputVector[i] << std::endl;
	}
	std::cout << "Num values written to output: " << nPrimitivesWritten << std::endl;

	std::cin.get();

	return 0;
}
