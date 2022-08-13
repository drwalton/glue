#include "SparseModel.hpp"
#include "files/Directories.hpp"
#include "graphics/FullScreenQuad.hpp"
#include <Eigen/Dense>
#include <fstream>
#include <array>
#include "math/KahanVal.hpp"
#include "coarsemodel/CoarseModelFile.hpp"
#include "coarsemodel/SparseModelVisibility.hpp"
#include "math/Points2D.hpp"

const size_t sparseTexWidth = 1024;
const size_t sparseTexHeight = 1024;

const float texBoundary = 0.05f;
const AARect floorTexRect{ 
	texBoundary, 0.5f - texBoundary, texBoundary, 0.5f - texBoundary };
const AARect ceilTexRect{ 
	0.5f + texBoundary, 1.f - texBoundary, texBoundary, 0.5f - texBoundary };
const AARect wallTexRect{ 
	texBoundary, 1.f - texBoundary, 0.5f + texBoundary, 1.f - texBoundary };

SparseModel::SparseModel(const std::string &sparseMeshFile)
	:texture(GLenum(GL_TEXTURE_2D), GL_RGBA,
		sparseTexWidth, sparseTexHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr),
	occlusionMaskTexture(GLenum(GL_TEXTURE_2D), GL_R8,
		sparseTexWidth, sparseTexHeight, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr),
	omniImDiffTexture(GLenum(GL_TEXTURE_2D), GL_RGBA,
		sparseTexWidth, sparseTexHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr),
	texturedMeshProgram(std::vector<std::string> {
		"TexturedMeshOffset.vert",
		"TexturedMeshOffset.frag"
	}),
	updateModelProgram(std::vector<std::string> {
		"UpdateModelTexture.vert",
		"UpdateModelTexture.frag"
	}),
	omniImDiffProgram(std::vector<std::string> {
			"OmniImDiff.vert",
				"OmniImDiff.frag"
		}),
	inpaintMethod(InpaintingMethod::CV_TELEA),
	ignoreOcclusion(false)
{
	bool e = stringEndsWith(sparseMeshFile, ".sparse_model");
	std::cout << "Loading sparse model from \"" << sparseMeshFile << "\"" << std::endl;;
	mesh.modelToWorld(this->modelToWorld());
	if (!e) {
		ModelLoader loader(sparseMeshFile);
		mesh.fromModelLoader(loader);
		findFaces(loader);
		modelType = SparseModelType::QUAD_MESH;
		std::cout << "\tModel type: GENERAL QUAD MESH" << std::endl;
	} else {
		loadCoarseModelFromFile(sparseMeshFile,
			&floorplan, &floorheight, &ceilingheight);
		modelType = SparseModelType::PRISM;
		std::cout << "\tModel type: PRISM" << std::endl;

		floorTexCoords = rescale2dVertsToRect(floorplan, floorTexRect);
		ceilTexCoords = rescale2dVertsToRect(floorplan, ceilTexRect);
		wallTexYMax = wallTexRect.maxY;
		wallTexYMin = wallTexRect.minY;
		wallTexX = findWallTexX(floorplan, wallTexRect.minX, wallTexRect.maxX);

		ModelLoader sparseMesh;
		//Add tex coords
		sparseMesh.texCoords().insert(sparseMesh.texCoords().end(), floorTexCoords.begin(), floorTexCoords.end());
		sparseMesh.texCoords().insert(sparseMesh.texCoords().end(), ceilTexCoords.begin(), ceilTexCoords.end());
		for (float x : wallTexX) {
			sparseMesh.texCoords().push_back(vec2(x, wallTexRect.minY));
		}
		for (float x : wallTexX) {
			sparseMesh.texCoords().push_back(vec2(x, wallTexRect.maxY));
		}
		//Add vertices.
		for (vec2 &v : floorplan) {
			sparseMesh.vertices().push_back(vec3(v.x(), floorheight, v.y()));
		}
		for (vec2 &v : floorplan) {
			sparseMesh.vertices().push_back(vec3(v.x(), ceilingheight, v.y()));
		}
		for (vec2 &v : floorplan) {
			sparseMesh.vertices().push_back(vec3(v.x(), floorheight, v.y()));
		}
		sparseMesh.vertices().push_back(vec3(floorplan[0].x(), floorheight, floorplan[0].y()));
		for (vec2 &v : floorplan) {
			sparseMesh.vertices().push_back(vec3(v.x(), ceilingheight, v.y()));
		}
		sparseMesh.vertices().push_back(vec3(floorplan[0].x(), ceilingheight, floorplan[0].y()));
		
		std::vector<GLuint> indicesFloor = triangulatePolygonIndices(floorplan);
		for (size_t i = 0; i < indicesFloor.size(); i += 3) {
			sparseMesh.indices().push_back(indicesFloor[i + 0]);
			sparseMesh.indices().push_back(indicesFloor[i + 2]);
			sparseMesh.indices().push_back(indicesFloor[i + 1]);
		}
		for (size_t i = 0; i < indicesFloor.size(); i += 3) {
			sparseMesh.indices().push_back(floorplan.size() + indicesFloor[i + 0]);
			sparseMesh.indices().push_back(floorplan.size() + indicesFloor[i + 1]);
			sparseMesh.indices().push_back(floorplan.size() + indicesFloor[i + 2]);
		}
		GLuint o = 2 * floorplan.size();
		for (size_t i = 0; i < wallTexX.size()-1; ++i) {
			GLuint i0 = o + i;
			GLuint i1 = o + i + 1;
			GLuint i2 = o + i + wallTexX.size();
			GLuint i3 = o + i + wallTexX.size() + 1;
			sparseMesh.indices().push_back(i0);
			sparseMesh.indices().push_back(i2);
			sparseMesh.indices().push_back(i1);

			sparseMesh.indices().push_back(i1);
			sparseMesh.indices().push_back(i2);
			sparseMesh.indices().push_back(i3);
		}

		//For testing, save mesh to file.
		sparseMesh.saveFile(getModelDir() + "sparseModelConvertedMesh.ply");
		mesh.fromModelLoader(sparseMesh);
		//TODO find faces.
	}
	vis = SparseModelVisibility::create(this);

	glGenFramebuffers(1, &framebufferUpdate);
	glGenRenderbuffers(1, &renderbufferUpdate);
	glGenFramebuffers(1, &framebufferDiff);
	glGenRenderbuffers(1, &renderbufferDiff);
	glGenFramebuffers(1, &framebufferInpaint);
	glGenRenderbuffers(1, &renderbufferInpaint);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferUpdate);
	glBindRenderbuffer(GL_RENDERBUFFER, renderbufferUpdate);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_RENDERBUFFER, renderbufferUpdate);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferDiff);
	glBindRenderbuffer(GL_RENDERBUFFER, renderbufferDiff);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_RENDERBUFFER, renderbufferDiff);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferInpaint);
	glBindRenderbuffer(GL_RENDERBUFFER, renderbufferInpaint);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_RENDERBUFFER, renderbufferUpdate);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	//Initially fill sparse texture and mask with test data.
	//Mask is set to 255 everywhere, sparse set to green, zero alpha.
	std::vector<unsigned char> sparseTexTestData, maskTestData;
	sparseTexTestData.resize(sparseTexHeight*sparseTexWidth * 4);
	maskTestData.resize(sparseTexHeight*sparseTexWidth);
	for (size_t i = 0; i < sparseTexTestData.size(); ++i) {
		if (i % 4 != 3) {
			sparseTexTestData[i] = 50;
		} else {
			sparseTexTestData[i] = 0;
		}
		if (i % 4 == 0) {
			maskTestData[i / 4] = 255;
		}
	}
	texture.update(sparseTexTestData.data());
	occlusionMaskTexture.update(maskTestData.data());

	//std::string facesFilename =
	//	sparseMeshFile.substr(0, sparseMeshFile.length() - 4) + ".txt";
	//std::ifstream facesFile (facesFilename);
	//while(!facesFile.eof()) {
	//	Face face;
	//	float x, y, w, h;
	//	facesFile >> x >> y >> w >> h;
	//	face.height = h*sparseTexHeight;
	//	face.width = w*sparseTexWidth;
	//	face.xoffset = x*sparseTexWidth;
	//	face.yoffset = y*sparseTexHeight;
	//	faces.push_back(face);
	//}
}

SparseModel::~SparseModel() throw()
{
	glDeleteFramebuffers(1, &framebufferUpdate);
	glDeleteRenderbuffers(1, &renderbufferUpdate);
	glDeleteFramebuffers(1, &framebufferDiff);
	glDeleteRenderbuffers(1, &renderbufferDiff);
	glDeleteFramebuffers(1, &framebufferInpaint);
	glDeleteRenderbuffers(1, &renderbufferInpaint);
}

void SparseModel::render()
{
	mesh.shaderProgram(&texturedMeshProgram);
	texture.bindToImageUnit(1);
	texturedMeshProgram.setUniform("tex", 1);
	texturedMeshProgram.setUniform("brightnessOffset", currBrightnessOffset);
	mesh.render();
}

void SparseModel::render(Texture *tex)
{
	mesh.shaderProgram(&texturedMeshProgram);
	tex->bindToImageUnit(1);
	texturedMeshProgram.setUniform("tex", 1);
	texturedMeshProgram.setUniform("brightnessOffset", currBrightnessOffset);
	mesh.render();
}

void SparseModel::updateTexture(Texture *omniImTexture, Texture *omniValidTexture, const mat4 &omniCamPose)
{
	renderOmniImDiff(omniImTexture, omniValidTexture, omniCamPose);

	mesh.shaderProgram(&updateModelProgram);

	if (!ignoreOcclusion) {
		computeOcclusionMask(omniCamPose);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, framebufferUpdate);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, texture.tex(), 0);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		throw std::runtime_error("FRAMEBUFFER NOT COMPLETE");
	}

	updateModelProgram.use();
	occlusionMaskTexture.bindToImageUnit(1);
	updateModelProgram.setUniform("mask", 1);

	omniImTexture->bindToImageUnit(2);
	updateModelProgram.setUniform("omniImTex", 2);
	
	omniValidTexture->bindToImageUnit(3);
	updateModelProgram.setUniform("omniImMask", 3);

	updateModelProgram.setUniform("cameraPose", omniCamPose);
	updateModelProgram.setUniform("brightnessOffset", currBrightnessOffset);

	glViewport(0,0,sparseTexWidth, sparseTexHeight);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	mesh.render();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	/*
	//TEST
	cv::Mat sparseTex(sparseTexHeight, sparseTexWidth, CV_8UC4);
	texture.getData(sparseTex.data, sparseTexHeight*sparseTexWidth * 4);
	cv::imshow("SPARSE TEX", sparseTex);
	cv::waitKey();
	*/
}

void SparseModel::renderOmniImDiff(Texture *omniImTexture, Texture *omniValidTexture, const mat4 &omniCamPose)
{
	mesh.shaderProgram(&omniImDiffProgram);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferDiff);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, omniImDiffTexture.tex(), 0);
	glViewport(0, 0, omniImDiffTexture.width(), omniImDiffTexture.height());

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE) {
		throw std::runtime_error("  FRAMEBUFFER NOT COMPLETE");
	}
	glClearColor(0.f, 0.f, 0.f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT);

	occlusionMaskTexture.bindToImageUnit(1);
	omniImDiffProgram.setUniform("mask", 1);

	omniImTexture->bindToImageUnit(2);
	omniImDiffProgram.setUniform("omniImTex", 2);

	texture.bindToImageUnit(3);
	omniImDiffProgram.setUniform("sparseRoomTex", 3);
	
	omniValidTexture->bindToImageUnit(4);
	omniImDiffProgram.setUniform("omniValidTex", 4);

	omniImDiffProgram.use();
	omniImDiffProgram.setUniform("cameraPose", omniCamPose);

	glViewport(0,0,sparseTexWidth, sparseTexHeight);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	mesh.render();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	cv::Mat testMat(sparseTexHeight, sparseTexWidth, CV_8UC4);
	omniImDiffTexture.getData(testMat.data, testMat.rows*testMat.cols * 4 * sizeof(unsigned char));

	/*
	//TEST CODE: show image
	cv::imshow("TEST", testMat);
	cv::waitKey();
	*/

	KahanVal<float> sumDiff(0.f);
	for (size_t r = 0; r < sparseTexHeight; ++r) {
		for (size_t c = 0; c < sparseTexWidth; ++c) {
			cv::Vec4b currVal = testMat.at<cv::Vec4b>(int(r), int(c));
			if (currVal[3] != 0) {
				sumDiff += float(currVal[0]) - float(currVal[2]);
			}
		}
	}
	float avgDiff = sumDiff.get() / (float(sparseTexHeight) * float(sparseTexWidth));
	currBrightnessOffset = avgDiff / 255.f;
}

typedef std::array<GLuint, 3> TriangleIndices;
typedef std::vector<TriangleIndices> FaceIndices;
typedef std::vector<FaceIndices> FaceIndexList;

bool trianglesAreAdjoining(TriangleIndices t1, TriangleIndices t2)
{
	size_t noInCommon = 0;
	for (size_t i1 = 0; i1 < 3; ++i1) {
		for (size_t i2 = i1; i2 < 3; ++i2) {
			if (t1[i1] == t2[i2]) {
				++noInCommon;
				break;
			}
		}
	}
	return noInCommon >= 2;
}

vec3 findTriNormal(TriangleIndices &t, const std::vector<vec3> &vertices) 
{
	vec3 t0 = vertices[t[0]];
	vec3 t1 = vertices[t[1]];
	vec3 t2 = vertices[t[2]];

	return (t1 - t0).cross(t2 - t0).normalized();
}

bool trianglesAreCoplanar(TriangleIndices &t1, TriangleIndices &t2, const std::vector<vec3> &vertices)
{
	vec3 n1 = findTriNormal(t1, vertices);
	vec3 n2 = findTriNormal(t2, vertices);
	return fabsf(n1.dot(n2)) > 0.99f;
}


bool equal(TriangleIndices &t1, TriangleIndices &t2)
{
	return t1[0] == t2[0] && t1[1] == t2[1] && t1[2] == t2[2];
}

bool triangleInFace(TriangleIndices &t, FaceIndices &f) {
	for (TriangleIndices &i : f) {
		if (equal(i, t)) {
			return true;
		}
	}
	return false;
}

void SparseModel::findFaces(const ModelLoader &loader)
{
	FaceIndexList faceList;
	//For each triangle in the mesh
	for (size_t i = 0; i < loader.indices().size(); i += 3) {
		TriangleIndices currTri = {loader.indices()[i], loader.indices()[i+1], loader.indices()[i+2]};
		FaceIndices *currFace = nullptr;
		//Load current face, if present.
		for (FaceIndices &face : faceList) {
			if (triangleInFace(currTri, face)) {
				currFace = &face;
				break;
			}
		}
		if (currFace == nullptr) {
			//If no existing face, add a new one.
			faceList.push_back(FaceIndices());
			currFace = &faceList.back();
		}

		//For each other triangle
		for (size_t i = 0; i < loader.indices().size(); i += 3) {
			TriangleIndices otherTri = {loader.indices()[i], loader.indices()[i+1], loader.indices()[i+2]};
			//Check if other triangle is already in the current face.
			if (triangleInFace(otherTri, *currFace)) {
				//Skip to next one - these triangles are already in a common face.
				continue;
			}

			//Check if triangle adjoining (i.e. two indices in common).
			if (trianglesAreAdjoining(currTri, otherTri)) {
				if (trianglesAreCoplanar(currTri, otherTri, loader.vertices())) {
					currFace->push_back(otherTri);
				}
			}
		}
	}

	//Final merge (Only necessary for faces consisting of 3 or more triangles (can skip for now).
	//TODO

	//TODO Debug
	//cv::Mat debugShowFaceMat(sparseTexHeight, sparseTexWidth, CV_8UC3);
	//debugShowFaceMat.setTo(cv::Vec3b(0, 0, 0));
	for (FaceIndices &face : faceList) {
		float minX = 1.f, minY = 1.f, maxX = 0.f, maxY = 0.f;
		for (TriangleIndices &tri : face) {
			for (GLuint i : tri) {
				vec2 tex = loader.texCoords()[i];
				if (tex.x() < minX) minX = tex.x();
				if (tex.y() < minY) minY = tex.y();
				if (tex.x() > maxX) maxX = tex.x();
				if (tex.y() > maxY) maxY = tex.y();
			}
		}
		Face f;
		f.xoffset = GLint(minX * sparseTexWidth);
		f.yoffset = GLint(minY * sparseTexHeight);
		f.width = GLsizei((maxX - minX) * sparseTexWidth);
		f.height = GLsizei((maxY - minY) * sparseTexHeight);
		faces.push_back(f);

		//cv::Vec3b color(rand()%255, rand()%255, rand()%255);
		//cv::rectangle(debugShowFaceMat, cv::Rect(f.xoffset, f.yoffset, f.width, f.height), color);
	}
	//cv::imshow("FACES", debugShowFaceMat);
	//cv::waitKey(0);
}

void SparseModel::inpaintFace(size_t faceIdx)
{
	Face face = faces[faceIdx];
	cv::Mat faceMat(cv::Size(face.width, face.height), CV_8UC4);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferInpaint);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, texture.tex(), 0);
	glReadPixels(face.xoffset, face.yoffset, face.width, face.height,
		GL_BGRA, GL_UNSIGNED_BYTE, faceMat.data);

	inpaintRbga8Mat(&faceMat, inpaintMethod);

	glBindTexture(GL_TEXTURE_2D, texture.tex());
	glTexSubImage2D(GL_TEXTURE_2D, 0, face.xoffset, face.yoffset,
		face.width, face.height, GL_BGRA, GL_UNSIGNED_BYTE, faceMat.data);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void SparseModel::inpaintFaces()
{
	for (size_t i = 0; i < faces.size(); ++i) {
		inpaintFace(i);
	}
}



void SparseModel::computeOcclusionMask(const mat4 &currCamPose)
{
	vis->computeVisibilityMask(currCamPose);
}

void SparseModel::debugShowOcclusionMask()
{
	cv::Mat occlusionMask(sparseTexHeight, sparseTexWidth, CV_8UC1);
	occlusionMask.setTo(0);
	occlusionMaskTexture.getData(occlusionMask.data, sparseTexWidth*sparseTexHeight);
	cv::imshow("Occlusion mask", occlusionMask);
	cv::imwrite(getModelDir() + "sparseModelVis/occlusionMask.png", occlusionMask);
	cv::waitKey(1);
}
