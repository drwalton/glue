#include "glue/WaveMesh.hpp"
#include "glue/Constants.hpp"
#include "glue/Directories.hpp"
#include "glue/RenderDoc.hpp"
#include <iostream>

const size_t gridWidth = 512;
const size_t gridHeight = 512;
const float meshWidth = 1.f;
const float meshHeight = 1.f;

namespace glue {

WaveMesh::WaveMesh(const glue::mat4 &m)
{
	modelToWorld(m);
	
	float left = -(meshWidth * 0.5f);
	float bottom = (meshHeight * 0.5f);
	float horzSpace = meshWidth / (float)(gridWidth);
	float vertSpace = meshHeight / (float)(gridHeight);
	std::vector<glue::vec4> verts(gridWidth*gridHeight);
	std::vector<glue::vec4> norms(gridWidth*gridHeight);
	for(size_t r = 0; r < gridHeight; ++r) {
		for(size_t c = 0; c < gridWidth; ++c) {
			verts[r*gridWidth + c] = vec4(left + horzSpace*c, 0.f, bottom - vertSpace*r, 1.f);
			norms[r*gridWidth + c] = vec4(0.f, 1.f, 0.f, 1.f);
		}
	}
	
	std::vector<GLuint> indices;
	for(size_t r = 0; r < gridHeight-1; ++r) {
		for(size_t c = 0; c < gridWidth-1; ++c) {
			indices.push_back(r*gridWidth + c);
			indices.push_back(r*gridWidth + c + 1);
			indices.push_back((r+1)*gridWidth + c);
			
			indices.push_back(r*gridWidth + c + 1);
			indices.push_back((r+1)*gridWidth + c + 1);
			indices.push_back((r+1)*gridWidth + c);
		}
	}
	nElems_ = indices.size();
	GLuint maxI = 0;
	for (GLuint &i : indices) {
		if (i > maxI) maxI = i;
	}
	std::cout << "max index" << maxI << std::endl;
	
	renderProgram_.reset(new ShaderProgram({
		GLUE_SHADER_DIR + "WaveMeshRender.vert",
		GLUE_SHADER_DIR + "WaveMeshRender.frag"
	}));
	updateProgram_.reset(new ShaderProgram({
		GLUE_SHADER_DIR + "WaveMeshUpdate.comp"
	}));
	
	vert_.reset(new VertexBuffer(verts, GL_DYNAMIC_DRAW));
	norm_.reset(new VertexBuffer(norms, GL_DYNAMIC_DRAW));
	elem_.reset(new ElementBuffer(indices, GL_STATIC_DRAW));
	
	vao_.bind();
	norm_->bind();
	glEnableVertexAttribArray(UniformLocation::NORM);
	glVertexAttribPointer(UniformLocation::NORM, 4, GL_FLOAT, GL_FALSE, 0, 0);
	vert_->bind();
	glEnableVertexAttribArray(UniformLocation::VERT);
	glVertexAttribPointer(UniformLocation::VERT, 4, GL_FLOAT, GL_FALSE, 0, 0);
	elem_->bind();
	vao_.unbind();
	throwOnGlError();
}

WaveMesh::~WaveMesh() throw()
{}

void WaveMesh::render()
{
	vao_.bind();
	renderProgram_->use();
	renderProgram_->setUniform("modelToWorld", modelToWorld());
	renderProgram_->setUniform("normToWorld", normToWorld());
	renderProgram_->setupCameraBlock();
	glDrawElements(GL_TRIANGLES, GLsizei(nElems_), GL_UNSIGNED_INT, 0);
	renderProgram_->unuse();
	vao_.unbind();
}

void WaveMesh::update()
{
	updateProgram_->use();
	vert_->bind();
	glEnableVertexAttribArray(UniformLocation::VERT);
	glVertexAttribPointer(UniformLocation::VERT, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vert_->get()); // Buffer Binding 1
	norm_->bind();
	glEnableVertexAttribArray(UniformLocation::NORM);
	glVertexAttribPointer(UniformLocation::NORM, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, norm_->get()); // Buffer Binding 1
	static float time = 0.f;
	updateProgram_->setUniform("time", time);
	updateProgram_->setUniform("height", 0.2f);
	updateProgram_->setUniform("freq", 8.f);
	time += 0.01f;
	glDispatchCompute(gridWidth, gridHeight, 1);
	updateProgram_->unuse();
}

}
