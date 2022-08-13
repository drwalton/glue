#include "glue/Bake.hpp"
#include "glue/GLWindow.hpp"
#include "glue/ShaderProgram.hpp"
#include "glue/SphericalHarmonics.hpp"
#include "glue/Directories.hpp"
#include "glue/Texture.hpp"
#include "glue/Mesh.hpp"
#include "glue/CubemapLookup.hpp"
#include "glue/KahanVal.hpp"
#include <nanort.h>
#include <iostream>
#include <random>
#include <thread>
#include <fstream>
#include <array>
#include <opencv2/opencv.hpp>
#ifdef _MSC_VER
#include <Windows.h>
#include <ShObjIdl.h>
#endif

namespace glue
{

const float RAY_MIN_T = 0.000001f;
const float RAY_MAX_T = 100.f;

void dilateVec4Image(vec4 *image, size_t width, size_t height)
{
	vec4 *tmpImage = new vec4[width*height];
	memcpy(tmpImage, image, width*height * sizeof(vec4));

	for (int r = 0; r < height; ++r) {
		for (int c = 0; c < width; ++c) {
			vec4 currPix = image[r*width + c];
			if (currPix[3] == 0.f) {
				vec4 avgSample = vec4::Zero();
				//above
				if (r >= 1) {
					avgSample += tmpImage[(r - 1)*width + c];
				}
				if (r < height-1) {
					avgSample += tmpImage[(r + 1)*width + c];
				}
				if (c >= 1) {
					avgSample += tmpImage[r*width + (c - 1)];
				}
				if (c < width-1) {
					avgSample += tmpImage[r*width + (c + 1)];
				}
				if (avgSample[3] > 0) {
					avgSample /= avgSample[3];
					image[r*width + c] = avgSample;
				}
			}
		}
	}

	delete[] tmpImage;
}

void renderWorldPosAndNormals(
	const std::vector<vec3>& vertices,
	const std::vector<vec3>& normals,
	const std::vector<vec2>& texCoords,
	const std::vector<GLuint>& indices,
	vec4 *worldPosImage,
	vec4 *normalImage,
	size_t width, size_t height)
{
	GLWindow win("Hidden", 0, 0);
	ShaderProgram renderWorldPosShader({
		GLUE_SHADER_DIR + "RenderWorldPosTex.vert",
		GLUE_SHADER_DIR + "RenderWorldPosTex.frag"
	});
	ShaderProgram renderNormalShader({
		GLUE_SHADER_DIR + "RenderNormalTex.vert",
		GLUE_SHADER_DIR + "RenderNormalTex.frag"
	});
	Mesh mesh;
	mesh.vert(vertices);
	mesh.norm(normals);
	mesh.tex(texCoords);
	mesh.elems(indices);
	Texture texture(GL_TEXTURE_2D, GL_RGBA32F, 
		GLsizei(width), GLsizei(height), 0, GL_RGBA, GL_FLOAT, nullptr, GL_NEAREST, GL_NEAREST);

	GLuint framebuffer;
	GLuint renderbuffer;
	glGenFramebuffers(1, &framebuffer);
	glGenRenderbuffers(1, &renderbuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.tex(), 0);
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		throw std::runtime_error("FRAMEBUFFER NOT COMPLETE");
	}
	glViewport(0, 0, GLsizei(width), GLsizei(height));
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	mesh.shaderProgram(&renderWorldPosShader);
	mesh.render();
	texture.getData(worldPosImage, width*height * sizeof(vec4));
	glClear(GL_COLOR_BUFFER_BIT);
	mesh.shaderProgram(&renderNormalShader);
	mesh.render();
	texture.getData(normalImage, width*height * sizeof(vec4));
	cv::Mat worldPosMat(cv::Size(int(width), int(height)), CV_32FC4,
		reinterpret_cast<unsigned char*>(worldPosImage));
	cv::imshow("Undilated world Pos.", worldPosMat);
	dilateVec4Image(worldPosImage, width, height);
	dilateVec4Image(worldPosImage, width, height);
	worldPosMat = cv::Mat(cv::Size(int(width), int(height)), CV_32FC4,
		reinterpret_cast<unsigned char*>(worldPosImage));
	cv::imshow("Dilated world Pos.", worldPosMat);
	cv::waitKey(1);
	dilateVec4Image(normalImage, width, height);
	dilateVec4Image(normalImage, width, height);

	glDeleteFramebuffers(1, &framebuffer);
	glDeleteRenderbuffers(1, &framebuffer);
}

SHProjection computePRTDiffuseShadowed(
	const vec3 &worldPos, const vec3 &normal,
	const nanort::BVHAccel<float> &accel,
	const nanort::TriangleIntersector<> &intersector,
	size_t nBands,
	size_t nSamples)
{
	SHProjection projection;
	for (int l = 0; l < nBands; ++l) {
		for (int m = -l; m <= l; ++m) {
			projection.push_back(0.0f);
		}
	}
	nanort::Ray<float> ray;
	memcpy(ray.org, worldPos.data(), sizeof(vec3));
	memcpy(ray.dir, normal.data(), sizeof(vec3));
	ray.min_t = RAY_MIN_T;
	ray.max_t = RAY_MAX_T;

	size_t sampleCount = 0;
	std::default_random_engine engine;
	std::uniform_real_distribution<float> dist(-1.f, 1.f);
	nanort::BVHTraceOptions traceOptions;

	//Generate samples via rejection sampling
	while (sampleCount < nSamples) {
		vec3 randSample;
		randSample.x() = dist(engine);
		randSample.y() = dist(engine);
		randSample.z() = dist(engine);
		if (randSample.squaredNorm() > 1.f) continue; //Not in sphere
		++sampleCount;
		if (randSample.dot(normal) <= 0.f) continue; //Not in normal hemisphere
		randSample.normalize();
		memcpy(ray.dir, randSample.data(), sizeof(vec3));
		nanort::TriangleIntersection<> isect;
		if (!accel.Traverse(ray, intersector, &isect, traceOptions)) {
			for (int l = 0; l < nBands; ++l) {
				for (int m = -l; m <= l; ++m) {
					float sh = realSH(l, m, randSample);
					if(sh != sh) throw 1;
					projection.at(l*(l + 1) + m) += sh;
				}
			}
		}
	}

	//Normalize
	for (float &i : projection) {
		i *= 4.0f * float(M_PI) / static_cast<float>(nSamples);
	}

	return projection;
}

SHProjection computePRTDiffuseInterreflected(
	const vec3 &worldPos, const vec3 &normal,
	const nanort::BVHAccel<float> &accel,
	const nanort::TriangleIntersector<> &intersector,
	size_t nBands,
	size_t nSamples,
	const cv::Mat &albedoTexture,
	const std::vector<vec2> &texCoords)
{
	SHProjection projection;
	for (int l = 0; l < nBands; ++l) {
		for (int m = -l; m <= l; ++m) {
			projection.push_back(0.0f);
		}
	}
	nanort::Ray<float> ray;
	memcpy(ray.org, worldPos.data(), sizeof(vec3));
	memcpy(ray.dir, normal.data(), sizeof(vec3));
	ray.min_t = RAY_MIN_T;
	ray.max_t = RAY_MAX_T;

	size_t sampleCount = 0;
	std::default_random_engine engine;
	std::uniform_real_distribution<float> dist(-1.f, 1.f);
	nanort::BVHTraceOptions traceOptions;

	//Generate samples via rejection sampling
	while (sampleCount < nSamples) {
		vec3 randSample;
		randSample.x() = dist(engine);
		randSample.y() = dist(engine);
		randSample.z() = dist(engine);
		if (randSample.squaredNorm() > 1.f) continue; //Not in sphere
		++sampleCount;
		if (randSample.dot(normal) <= 0.f) continue; //Not in normal hemisphere
		randSample.normalize();
		memcpy(ray.dir, randSample.data(), sizeof(vec3));
		nanort::TriangleIntersection<> isect;
		if (!accel.Traverse(ray, intersector, &isect)) {
			for (int l = 0; l < nBands; ++l) {
				for (int m = -l; m <= l; ++m) {
					float sh = realSH(l, m, randSample);
					if(sh != sh) throw 1;
					projection.at(l*(l + 1) + m) += sh;
				}
			}
		} else {
			//Intersection occured.
			
		}
	}

	//Normalize
	for (float &i : projection) {
		i *= 4.0f * float(M_PI) / static_cast<float>(nSamples);
	}

	return projection;
}

float computeAmbientOcclusion(
	const vec3 &worldPos, const vec3 &normal,
	const nanort::BVHAccel<float> &accel,
	const nanort::TriangleIntersector<> &intersector,
	size_t nSamples)
{
	nanort::Ray<float> ray;
	memcpy(ray.org, worldPos.data(), sizeof(vec3));
	memcpy(ray.dir, normal.data(), sizeof(vec3));
	ray.min_t = RAY_MIN_T;
	ray.max_t = RAY_MAX_T;

	size_t sampleCount = 0;
	float occlusion = 0.f;
	std::default_random_engine engine;
	std::uniform_real_distribution<float> dist(-1.f, 1.f);
	nanort::BVHTraceOptions traceOptions;

	//Generate samples via rejection sampling
	while (sampleCount < nSamples) {
		vec3 randSample;
		randSample.x() = dist(engine);
		randSample.y() = dist(engine);
		randSample.z() = dist(engine);
		if (randSample.squaredNorm() > 1.f) continue; //Not in sphere
		if (randSample.dot(normal) <= 0.f) continue; //Not in normal hemisphere
		++sampleCount;
		memcpy(ray.dir, randSample.data(), sizeof(vec3));
		nanort::TriangleIntersection<> isect;
		if (accel.Traverse(ray, intersector, &isect)) {
			occlusion += 1.f;
		}
	}

	return 1.f - (occlusion / nSamples);
}

template<typename F>
void parallelProcessImage(size_t width, size_t height,
	F func)
{
	int nThreads = std::thread::hardware_concurrency();
	std::vector<std::thread> threads(nThreads);
	std::cout << "Launching " << nThreads << " threads" << std::endl;
	int rowsPerThread = int(height) / nThreads;
	int startRow = 0, endRow = startRow + rowsPerThread;
	for (size_t t = 0; t < nThreads; ++t) {
		threads[t] = std::thread([&] { func(t, startRow, endRow); });
		startRow += rowsPerThread;
		endRow = startRow + rowsPerThread;
		if (t == nThreads - 1) {
			endRow = int(height);
		}
	}

	int nJoinedThreads = 0;
	while (nJoinedThreads < nThreads) {
		for (size_t i = 0; i < nThreads; ++i) { 
			if (threads[i].joinable()) {
				threads[i].join();
				++nJoinedThreads;
				std::cout << "Thread " << i << " finished: " << nThreads - nJoinedThreads << " remain" << std::endl;
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

void printUpdatesAt10Percents(size_t startRow, size_t endRow, size_t r) {
	static size_t currPercent = 0;
	size_t percent = ((r - startRow) * 10) / (endRow - startRow);
	if (percent > currPercent) {
		currPercent = percent;
		std::cout << percent * 10 << "% complete..." << std::endl;
	}
}
void printUpdatesAt2Percents(size_t startRow, size_t endRow, size_t r) {
	static size_t currPercent = 0;
	size_t percent = ((r - startRow) * 50) / (endRow - startRow);
	if (percent > currPercent) {
#ifdef _MSC_VER
		//Super hacky code
		static ITaskbarList3 *ptr = nullptr;
		static HWND hwnd;
		if (!ptr) {
			CoInitialize(NULL);
			CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_ITaskbarList3, (LPVOID*)&ptr);
			hwnd = GetConsoleWindow();
		}
		if (ptr->SetProgressState(hwnd, TBPF_NORMAL) != S_OK) {
			throw 1;
		}
		if (ptr->SetProgressValue(hwnd, percent * 2, 100) != S_OK) {
			throw 1;
		}
#else
		currPercent = percent;
		std::cout << percent * 2 << "% complete..." << std::endl;
#endif
	}
}

void bakeAmbientOcclusionTexture(
	const std::vector<vec3>& vertices,
	const std::vector<vec3>& normals,
	const std::vector<vec2>& texCoords,
	const std::vector<GLuint> &indices,
	float * outputTexture,
	vec4 *worldPosImage,
	vec4 *normalImage,
	size_t width, size_t height,
	size_t nSamples)
{
	//Make acceleration structure
	nanort::TriangleMesh<float> triangleMesh(
		reinterpret_cast<const float*>(vertices.data()),
		indices.data(), sizeof(vec3));
	nanort::TriangleSAHPred<float> trianglePred(
		reinterpret_cast<const float*>(vertices.data()),
		indices.data(), sizeof(vec3));
	nanort::BVHAccel<float> accel;
	nanort::BVHBuildOptions<float> options;
	accel.Build((unsigned int)(indices.size() / 3), triangleMesh, trianglePred, options);

	parallelProcessImage(width, height, [
		&vertices, &indices, width, &worldPosImage, 
		&normalImage, &outputTexture, &accel, nSamples]
		(size_t tid, size_t startRow, size_t endRow) {
		//Need unique intersector for each thread.
		nanort::TriangleIntersector<> intersector(
			reinterpret_cast<const float*>(vertices.data()), indices.data(), sizeof(vec3));
		for (size_t r = startRow; r < endRow; ++r) {
			if (tid == 0) {
				printUpdatesAt10Percents(startRow, endRow, r);
			}
			for (size_t c = 0; c < width; ++c) {
				vec4 worldPos = worldPosImage[r*width + c];
				if (worldPos.w() == 0.f) {
					outputTexture[r*width + c] = 0.f;
				}
				else {
					vec4 normalDir = normalImage[r*width + c];
					outputTexture[r*width + c] =
						computeAmbientOcclusion(
							worldPos.block<3, 1>(0, 0),
							normalDir.block<3, 1>(0, 0),
							accel, intersector, nSamples);
				}
			}
		}
	});
}

PrtCoeffts<float> bakeDiffuseShadowedPrtCoeffts(
	const std::vector<vec3>& vertices,
	const std::vector<vec3>& normals,
	const std::vector<vec2>& texCoords,
	const std::vector<GLuint>& indices,
	vec4 * worldPosImage, vec4 * normalImage,
	size_t width, size_t height,
	size_t nBands, size_t nSamples)
{
	//Make acceleration structure
	nanort::TriangleMesh<float> triangleMesh(
		reinterpret_cast<const float*>(vertices.data()),
		indices.data(), sizeof(vec3));
	nanort::TriangleSAHPred<float> trianglePred(
		reinterpret_cast<const float*>(vertices.data()),
		indices.data(), sizeof(vec3));
	nanort::BVHAccel<float> accel;
	nanort::BVHBuildOptions<float> options;
	accel.Build((const unsigned int)(indices.size() / 3), triangleMesh, trianglePred, options);

	PrtCoeffts<float> coeffts(width, height, nBands);
	parallelProcessImage(width, height, 
		[&vertices, &indices, width, height, &worldPosImage, &coeffts, &normalImage, &accel,
		nBands, nSamples]
		(size_t tid, int startRow, int endRow) {
		//Need unique intersector for each thread.
		nanort::TriangleIntersector<> intersector(
			reinterpret_cast<const float*>(vertices.data()), indices.data(), sizeof(vec3));
		for (size_t r = startRow; r < endRow; ++r) {
			if (tid == 0) {
				printUpdatesAt2Percents(startRow, endRow, r);
			}
			for (size_t c = 0; c < width; ++c) {
				vec4 worldPos = worldPosImage[r*width + c];
				if (worldPos.w() == 0.f) {
					for (float *t : coeffts.coeffts()) {
						t[r*width + c] = 0.f;
					}
				}
				else {
					vec4 normalDir = normalImage[r*width + c];
					SHProjection proj = computePRTDiffuseShadowed(
						worldPos.block<3, 1>(0, 0),
						normalDir.block<3, 1>(0, 0),
						accel, intersector, nBands, nSamples);
					for (size_t i = 0; i < coeffts.size(); ++i) {
						coeffts[i][r*width + c] = proj[i];
					}
				}
			}
		}

		if (tid == 0) {
			std::cout << "100% complete!" << std::endl;
		}

	});
	
	return coeffts;
}

PrtCoeffts<float> bakeDiffuseInterreflectedPrtCoeffts(
	const std::vector<vec3>& vertices,
	const std::vector<vec3>& normals,
	const std::vector<vec2>& texCoords,
	const std::vector<GLuint>& indices,
	vec4 * worldPosImage, vec4 * normalImage,
	const cv::Mat &albedoTexture,
	size_t width, size_t height,
	size_t nBands, size_t nSamples)
{
	//Make acceleration structure
	nanort::TriangleMesh<float> triangleMesh(
		reinterpret_cast<const float*>(vertices.data()),
		indices.data(), sizeof(vec3));
	nanort::TriangleSAHPred<float> trianglePred(
		reinterpret_cast<const float*>(vertices.data()),
		indices.data(), sizeof(vec3));
	nanort::BVHAccel<float> accel;
	nanort::BVHBuildOptions<float> options;
	accel.Build((const unsigned int)(indices.size() / 3), triangleMesh, trianglePred, options);

	PrtCoeffts<float> coeffts(width, height, nBands);
	parallelProcessImage(width, height, 
		[&vertices, &indices, width, &worldPosImage, &coeffts, &normalImage, &accel, nBands,
		nSamples, &albedoTexture, &texCoords]
		(size_t tid, int startRow, int endRow) {
		//Need unique intersector for each thread.
		nanort::TriangleIntersector<> intersector(
			reinterpret_cast<const float*>(vertices.data()), indices.data(), sizeof(vec3));
		for (size_t r = startRow; r < endRow; ++r) {
			if (tid == 0) {
				printUpdatesAt2Percents(startRow, endRow, r);
			}
			for (size_t c = 0; c < width; ++c) {
				vec4 worldPos = worldPosImage[r*width + c];
				if (worldPos.w() == 0.f) {
					for (float *t : coeffts.coeffts()) {
						t[r*width + c] = 0.f;
					}
				}
				else {
					vec4 normalDir = normalImage[r*width + c];
					SHProjection proj = computePRTDiffuseInterreflected(
						worldPos.block<3, 1>(0, 0),
						normalDir.block<3, 1>(0, 0),
						accel, intersector, nBands, nSamples,
						albedoTexture, texCoords);
					for (size_t i = 0; i < coeffts.size(); ++i) {
						coeffts[i][r*width + c] = proj[i];
					}
				}
			}
		}

		if (tid == 0) {
			std::cout << "100% complete!" << std::endl;
		}

	});
	
	return coeffts;
}


SHProjection3 projectCubemap(
	const std::array<cv::Mat, 6> &cubemap,
	size_t nSamples, size_t nBands)
{
	std::vector<KahanVal<glue::vec3>> projSums;
	for(size_t i = 0; i < calcNumSHCoeffts(nBands); ++i) {
		projSums.emplace_back(glue::vec3::Zero());
	}
	SHProjection3 projection;
	projection.resize(calcNumSHCoeffts(nBands));
	memset(projection.data(), 0, projection.size()*sizeof(vec3));
	
	size_t currNSamples = 0;
	std::default_random_engine engine;
	std::uniform_real_distribution<float> dist(-1.f, 1.f);
	while(currNSamples < nSamples) {
		vec3 randSample;
		randSample.x() = dist(engine);
		randSample.y() = dist(engine);
		randSample.z() = dist(engine);
		float sqNorm = randSample.squaredNorm();
		if (sqNorm > 1.f) continue; //Not in sphere
		randSample /= sqrtf(sqNorm);
		++currNSamples;
		vec3 sample = sampleCubemapGlCoordSpace(cubemap, randSample);
		for(int l = 0; l < nBands; ++l) {
			for(int m = -l; m <= l; ++m) {
				float shVal = realSH(l, m, randSample);
				projSums[l*(l+1) + m] += shVal * sample;
			}
		}
	}
	
	for(size_t i = 0; i < projection.size(); ++i) {
		projection[i] = projSums[i].get() * 4.f * float(M_PI) / float(nSamples);
	}
	return projection;
}


SHProjection3 projectCubemap(
	const std::array<cv::Mat, 6> &cubemap, size_t nBands)
{
	std::vector<KahanVal<glue::vec3>> projSums;
	KahanVal<float> weightSum(0.f);
	for(size_t i = 0; i < calcNumSHCoeffts(nBands); ++i) {
		projSums.emplace_back(glue::vec3::Zero());
	}
	//projSums.resize(calcNumSHCoeffts(nBands));
	SHProjection3 projection;
	projection.resize(calcNumSHCoeffts(nBands));
	memset(projection.data(), 0, projection.size()*sizeof(vec3));
	
	size_t currNSamples = 0;
	
	for(size_t face = 0; face < 6; ++face) {
		for(size_t row = 0; row < cubemap[0].rows; ++row) {
			for(size_t col = 0; col < cubemap[0].cols; ++col) {
				cv::Vec3b sampleRGB8;
				if(cubemap[face].channels() == 3) {
					sampleRGB8 = cubemap[face].at<cv::Vec3b>(int(row), int(col));
				} else if(cubemap[face].channels() == 4) {
					cv::Vec4b sampleRGBA8 = cubemap[face].at<cv::Vec4b>(int(row), int(col));
					for(size_t i = 0; i < 3; ++i) {
						sampleRGB8[int(i)] = sampleRGBA8[int(i)];
					}
				}
				float weight = getCubemapSampleWeight(row, col, cubemap[face].rows, cubemap[face].cols);
				weightSum += weight;
				glue::vec3 dir = cubemapPixelToDir(cubemap, row, col, face);
				dir = cubemapToGl * dir;
				glue::vec3 sample;
				for(size_t i = 0; i < 3; ++i) {
					sample[i] = float(sampleRGB8[int(i)]) / 255.f;
				}
				sample *= weight;
        		for(int l = 0; l < nBands; ++l) {
        			for(int m = -l; m <= l; ++m) {
        				float shVal = realSH(l, m, dir);
        				projSums[l*(l+1) + m] += shVal * sample;
        			}
        		}
				++currNSamples;
			}
		}
	}
	
	for(size_t i = 0; i < projection.size(); ++i) {
		projection[i] = projSums[i].get() * 4.f * float(M_PI) / (weightSum.get());
	}
	return projection;
}

void shToCubemap(const SHProjection3 & proj, std::array<cv::Mat, 6>* cubemap)
{
	for (size_t face = 0; face < 6; ++face) {
		for (size_t r = 0; r < (*cubemap)[0].rows; ++r) {
			for (size_t c = 0; c < (*cubemap)[0].cols; ++c) {
				vec3 dir = cubemapPixelToDir(*cubemap, r, c, face);
				vec3 SHVal = evaluateSH(proj, dir);
				(*cubemap)[face].at<cv::Vec3f>(int(r), int(c)) = 
					cv::Vec3f(SHVal.x(), SHVal.y(), SHVal.z());
			}
		}
	}
}

void viewPrtCoefftsUnderLighting(
	const PrtCoeffts<float> &prtCoeffts,
	const SHProjection3 &lighting)
{
	cv::Mat result(cv::Size(int(prtCoeffts.width()), int(prtCoeffts.height())), CV_32FC3);
	result.setTo(0);
	float max = -FLT_MAX;
	float avg = 0.f;
	for (size_t i = 0; i < prtCoeffts.size(); ++i) {
		for (size_t j = 0; j < prtCoeffts.width()*prtCoeffts.height(); ++j) {
			float prtVal = prtCoeffts[i][j];
			vec3 contribution = prtVal * lighting[i];
			if(contribution == contribution) {
    			((vec3*)(result.data))[j] += contribution;
    			max = fmaxf(max, contribution.x());
    			avg += contribution.x();
			}
		}
	}
	
	
	std::cout << "Max: " << max << std::endl;
	std::cout << "Avg: " << avg/float(prtCoeffts.width()*prtCoeffts.height()) << std::endl;
	cv::imshow("Texture under lighting", result / max);
	cv::waitKey();
}

}
