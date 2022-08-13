#include "PointCloudProjector.hpp"	
#include "GraphicsException.hpp"
#include "files/Directories.hpp"

AACuboid fitCuboidAroundPointCloud(const std::vector<vec3> &pointCloud,
	float boundarySize)
{
	AACuboid cuboid;
	cuboid.minX = cuboid.minY = cuboid.minZ = FLT_MAX;
	cuboid.maxX = cuboid.maxY = cuboid.maxZ = FLT_MIN;
	for(const vec3 &v : pointCloud) {
		if(v.x() < cuboid.minX) cuboid.minX = v.x();
		if(v.y() < cuboid.minY) cuboid.minY = v.y();
		if(v.z() < cuboid.minZ) cuboid.minZ = v.z();
		if(v.x() > cuboid.maxX) cuboid.maxX = v.x();
		if(v.y() > cuboid.maxY) cuboid.maxY = v.y();
		if(v.z() > cuboid.maxZ) cuboid.maxZ = v.z();
	}
	
	cuboid.minX -= boundarySize;
	cuboid.minY -= boundarySize;
	cuboid.minZ -= boundarySize;
	cuboid.maxX += boundarySize;
	cuboid.maxY += boundarySize;
	cuboid.maxZ += boundarySize;
	return cuboid;
}

PointCloudProjector::PointCloudProjector(
	size_t width2D, size_t height2D, size_t height1D)
	:texture1D(GL_TEXTURE_2D, GL_R32F, GLsizei(height1D), 1, 0,
		GL_RED, GL_FLOAT),
	texture2D(GL_TEXTURE_2D, GL_R32F, GLsizei(width2D), GLsizei(height2D), 0,
		GL_RED, GL_FLOAT),
	preProjTransform(mat4::Identity()),
	shaderProgram_({
		"PointCloudProjector.vert",
		"PointCloudProjector.frag"}),
	width2D_(width2D), height2D_(height2D), height1D_(height1D)
{
	glBindTexture(GL_TEXTURE_2D, texture1D.tex());
	glGenFramebuffers(1, &framebuffer1D_);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer1D_);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		texture1D.tex(), 0);
	GLenum drawbuffers[] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, drawbuffers);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		throw GraphicsException("1D Framebuffer Incomplete");
	}
	checkForGLError();
	
	glBindTexture(GL_TEXTURE_2D, texture2D.tex());
	glGenFramebuffers(1, &framebuffer2D_);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer2D_);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		texture1D.tex(), 0);
	glDrawBuffers(1, drawbuffers);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		throw GraphicsException("2D Framebuffer Incomplete");
	}
	checkForGLError();
	
	glGenVertexArrays(1, &vao_);
	
	glBindVertexArray(vao_);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

PointCloudProjector::~PointCloudProjector() throw()
{}

void PointCloudProjector::project1D(VertexBuffer &pointCloud, size_t nPoints)
{
	project(pointCloud, nPoints, false);
}

void PointCloudProjector::project2D(VertexBuffer &pointCloud, size_t nPoints)
{
	project(pointCloud, nPoints, true);
}

void PointCloudProjector::project(
	VertexBuffer &pointCloud, size_t nPoints, bool twod)
{
	
	glBindVertexArray(vao_);
	shaderProgram_.use();
	
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendEquation(GL_ADD);
	glBlendFunc(GL_ONE, GL_ONE);
	
	mat4 proj;
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer2D_);
	if(twod) {
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			texture2D.tex(), 0);
		glViewport(0, 0, GLsizei(width2D_), GLsizei(height2D_));
		proj = makeProjMatrix2D(projectionVolume);
	} else {
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			texture1D.tex(), 0);
		glViewport(0, 0, int(height1D_), 1);
		proj = makeProjMatrix1D(projectionVolume);
	}
	GLenum drawbuffers[] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, drawbuffers);
	
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);
	
	shaderProgram_.setUniform("preProj", preProjTransform);
	shaderProgram_.setUniform("proj", proj);
	
	pointCloud.bind();
	//Temp thing
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	shaderProgram_.validate();
	glDrawArrays(GL_POINTS, 0, GLsizei(nPoints));
	pointCloud.unbind();
	
	glBindFramebuffer(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT);
	shaderProgram_.unuse();
	glBindVertexArray(0);
	checkForGLError();
}

void PointCloudProjector::get1DProjIm(cv::Mat &m)
{
	getProjIm(m, texture1D, height1D_, 1);
}

void PointCloudProjector::get2DProjIm(cv::Mat &m)
{
	getProjIm(m, texture2D, width2D_, height2D_);
}

void PointCloudProjector::getProjIm(cv::Mat &m, Texture &texture, size_t w, size_t h)
{
	if (m.size() != cv::Size(int(w), int(h)) || m.type() != CV_32FC1) {
		m = cv::Mat(cv::Size(int(w), int(h)), CV_32FC1);
	}
	glClampColor(GL_CLAMP_READ_COLOR, GL_FALSE);
	glClampColor(GL_CLAMP_VERTEX_COLOR, GL_FALSE);
	glClampColor(GL_CLAMP_FRAGMENT_COLOR, GL_FALSE);
	glBindTexture(GL_TEXTURE_2D, texture.tex());
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, m.data);
	glBindTexture(GL_TEXTURE_2D, 0);
}

mat4 PointCloudProjector::makeProjMatrix1D(const AACuboid &p)
{
	//TODO double check these matrices
	mat4 proj;
	proj <<
		0.f, 2.f/(p.maxY - p.minY), 0.f, -(p.maxY + p.minY) / (p.maxY - p.minY),
		0.f, 0.f, 0.f, 0.f,
		0.f, 0.f, 0.f, 0.f,
		0.f, 0.f, 0.f, 1.f;
	return proj;
}

mat4 PointCloudProjector::makeProjMatrix2D(const AACuboid &p)
{
	//TODO double check these matrices
	mat4 proj;
	proj <<
		2.f/(p.maxX - p.minX), 0.f, 0.f, -(p.maxX + p.minX) / (p.maxX - p.minX),
		0.f, 0.f, 2.f/(p.maxZ - p.minZ), -(p.maxZ + p.minZ) / (p.maxZ - p.minZ),
		0.f, 2.f/(p.maxY - p.minY), 0.f, -(p.maxY + p.minY) / (p.maxY - p.minY),
		0.f, 0.f, 0.f, 1.f;
	return proj;
}
