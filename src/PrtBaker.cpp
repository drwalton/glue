#define _USE_MATH_DEFINES
#include "glue/PrtBaker.hpp"

#include "glue/Bake.hpp"
#include "glue/KahanVal.hpp"
#include "glue/SphericalHarmonics.hpp"

#include <nanort.h>
#include <random>
#include <thread>
#include <cmath>

const float ONE_OVER_PI = float(1.0 / M_PI);
const float AREA_OF_SPHERE = float(4.0 * M_PI);
const float RAY_MIN_T = 0.000001f;
const float RAY_MAX_T = 100.f;

namespace glue
{

//!\brief Take a sample from a 2D image composed of PRT coefficients.
//!\note Coordinates behave similarly to sampling a texture in OpenGL.
//!\param texCoord Texture coordinates (in range [0,1]^2)
std::vector<vec3> samplePrtCoeffts(const PrtCoeffts<vec3> &coeffts, const vec2 &texCoord)
{
	std::vector<vec3> output(coeffts.size());

	vec2 pixelCoord(
		texCoord.x() * float(coeffts.width()), 
		float(coeffts.height()) - texCoord.y() * float(coeffts.height()));
	int colLeft = size_t(floorf(pixelCoord.x() - 0.5f));
	if (colLeft < 0) colLeft = 0;
	int colRight = colLeft + 1;
	if (colRight > coeffts.width() - 1) colRight = coeffts.width() - 1;
	int rowAbove = size_t(floorf(pixelCoord.y() - 0.5f));
	if (rowAbove < 0) rowAbove = 0;
	int rowBelow = rowAbove + 1;
	if (rowBelow > coeffts.height() - 1) rowBelow = coeffts.height() - 1;
	float distFromLeft = texCoord.x() - 0.5f - floorf(texCoord.x() - 0.5f);
	float distFromTop = texCoord.y() - 0.5f - floorf(texCoord.y() - 0.5f);

	for (size_t i = 0; i < coeffts.size(); ++i) {
		vec3 sampleTL = coeffts[i][colLeft + rowAbove*coeffts.width()];
		vec3 sampleBL = coeffts[i][colLeft + rowBelow*coeffts.width()];
		vec3 sampleTR = coeffts[i][colRight + rowAbove*coeffts.width()];
		vec3 sampleBR = coeffts[i][colRight + rowBelow*coeffts.width()];

		 output[i] =
			sampleTL * (1.f - distFromLeft) * (1.f - distFromTop) +
			sampleTR * distFromLeft * (1.f - distFromTop) +
			sampleBL * (1.f - distFromLeft) * distFromTop +
			sampleBR * distFromLeft * distFromTop;
	}

	return output;
}

//!\brief Process an image in parallel, with each thread processing a horizontal
//!       slice of the image.
//!\note This implementation launches twice as many threads as the no. of cores
//!      available. This provides better performance when not all slices take
//!      the same time to process.
template<typename F>
void parallelProcessImage(size_t width, size_t height,
	F func)
{
	int nThreads = std::thread::hardware_concurrency()*2;
	std::vector<std::thread> threads(nThreads);
	std::cout << "Launching " << nThreads << " threads" << std::endl;
	int rowsPerThread = int(height) / nThreads;
	int startRow = 0, endRow = startRow + rowsPerThread;
	for (size_t t = 0; t < nThreads; ++t) {
		threads[t] = std::thread([rowsPerThread, &func, t, startRow, endRow] { func(t, startRow, endRow); });
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

struct SpecularMesh {
	nanort::TriangleMesh<float> mesh;
	nanort::TriangleSAHPred<float> pred;
	nanort::BVHAccel<float> accel;
	std::vector<vec3> norms;

	SpecularMesh(const std::vector<vec3>& meshVerts, const std::vector<vec3>& meshNorms, const std::vector<GLuint>& meshIndices)
		:mesh((const float*)meshVerts.data(), meshIndices.data(), sizeof(vec3)),
		pred((const float*)meshVerts.data(), meshIndices.data(), sizeof(vec3)),
		norms(meshNorms)
	{
		accel.Build((unsigned int)(meshIndices.size()/3), mesh, pred);
	}
};

struct PrtBaker::Impl {

	nanort::TriangleMesh<float> mesh;
	nanort::TriangleSAHPred<float> pred;
	nanort::BVHAccel<float> accel;
	std::vector<vec2> texCoords;
	std::unique_ptr<SpecularMesh> specularMesh;

	Impl(
	const std::vector<vec3> &meshVerts,
	const std::vector<vec3> &meshNorms,
	const std::vector<vec2> &meshTexCoords,
	const std::vector<GLuint> &meshIndices)
		:mesh((const float*)meshVerts.data(), meshIndices.data(), sizeof(vec3)),
		pred((const float*)meshVerts.data(), meshIndices.data(), sizeof(vec3))
	{
		texCoords = meshTexCoords;
		accel.Build((unsigned int)(meshIndices.size()/3), mesh, pred);
	}
};

PrtBaker::PrtBaker(size_t nBands, size_t nSamples)
	:nBands_(nBands)
{
	//Precompute quasi-random directions, sh coeffts
	size_t nCoeffts = calcNumSHCoeffts(nBands);

	std::default_random_engine engine;
	std::uniform_real_distribution<float> dist(-1.f, 1.f);
	vec3 randDir;
	
	while(sampleDirs_.size() < nSamples) {
		randDir.x() = dist(engine);
		randDir.y() = dist(engine);
		randDir.z() = dist(engine);
		float sqNorm = randDir.squaredNorm();
		if(sqNorm <= 1.f) {
			randDir /= sqrtf(sqNorm);
			sampleDirs_.push_back(randDir);
		}
	}

	for(auto &dir : sampleDirs_) {
		std::vector<float> coeffts(nCoeffts);

		for(int l = 0; l < nBands; ++l) {
			for(int m = -l; m <= l; ++m) {
				coeffts[l*(l+1) + m] = realSH(l, m, dir);
			}
		}

		sampleShVals_.push_back(coeffts);
	}
}

PrtBaker::~PrtBaker() throw()
{}


void PrtBaker::setupMesh(
	const std::vector<vec3> &meshVerts,
	const std::vector<vec3> &meshNorms,
	const std::vector<vec2> &meshTexCoords,
	const std::vector<GLuint> &meshIndices,
	size_t width, size_t height)
{
	width_ = width;
	height_ = height;
	worldPosTex_.reset(new vec4[width*height]);
	normalTex_.reset(new vec4[width*height]);

	renderWorldPosAndNormals(meshVerts, meshNorms, meshTexCoords, meshIndices,
		worldPosTex_.get(), normalTex_.get(), width, height);

	pimpl_.reset(new Impl(meshVerts, meshNorms, meshTexCoords, meshIndices));
}

void PrtBaker::bakeDiffuseShadowedPrt(PrtCoeffts<vec3> *out, vec3 *albedoTex)
{
	parallelProcessImage(width_, height_, 
		[this, out, albedoTex] (size_t tid, size_t startRow, size_t endRow) {

		nanort::Ray<float> ray;
		nanort::TriangleIntersector<float> intersector(
			pimpl_->mesh.vertices_, pimpl_->mesh.faces_, sizeof(vec3));
		ray.min_t = RAY_MIN_T;
		ray.max_t = RAY_MAX_T;

		for(size_t r = startRow; r < endRow; ++r) {
			for(size_t c = 0; c < width_; ++c) {
				vec4 worldPos = worldPosTex_[r*width_ + c];
				if(worldPos.w() == 0) {
					continue;
				}
				vec3 norm = normalTex_[r*width_ + c].block<3,1>(0,0).normalized();
				memcpy(ray.org, worldPos.data(), sizeof(vec3));
				std::vector<KahanVal<vec3> > coefftSums;
				for (size_t j = 0; j < calcNumSHCoeffts(nBands_); ++j) {
					coefftSums.emplace_back(vec3::Zero());
				}

				for(size_t i = 0; i < sampleDirs_.size(); ++i) {
					float dotProd = sampleDirs_[i].dot(norm);
					if(dotProd <= 0.f) {
						continue;
					}
					memcpy(ray.dir, sampleDirs_[i].data(), sizeof(vec3));
					nanort::TriangleIntersection<> intersection;
					if(!pimpl_->accel.Traverse(ray, intersector, &intersection)){
						for (size_t j = 0; j < coefftSums.size(); ++j) {
							coefftSums[j] += vec3::Ones() * dotProd * sampleShVals_[i][j];
						}
					}
				}
				vec3 albedo = albedoTex[r*width_ + c];

				for (size_t j = 0; j < coefftSums.size(); ++j) {
					(*out)[j][r*width_ + c] = 4.f * M_PI *
						(albedo.array() * coefftSums[j].get().array()).matrix() / sampleDirs_.size();
						//Coefft-wise multiply
				}
			}
		}

	});
}

void PrtBaker::bakeDiffuseInterreflectedPrt(
	size_t nBounces, PrtCoeffts<vec3> *out, vec3 *albedoTex)
{
	bakeDiffuseShadowedPrt(out, albedoTex);
	for(size_t i = 1; i <= nBounces; ++i) {
		std::cout << "Starting bounce " << i << std::endl;
		performInterreflectionPass(out, albedoTex);
		std::cout << "Bounce " << i << " completed." << std::endl;
	}
}

void PrtBaker::setupSpecularMesh(
	const std::vector<vec3>& meshVerts, 
	const std::vector<vec3>& meshNorms, 
	const std::vector<GLuint>& meshIndices)
{
	pimpl_->specularMesh.reset(new SpecularMesh(meshVerts, meshNorms, meshIndices));
}

void PrtBaker::performInterreflectionPass(PrtCoeffts<vec3> *out, vec3 *albedoTex)
{
	PrtCoeffts<vec3> prevBounce(*out);
	
	parallelProcessImage(width_, height_, 
		[this, out, albedoTex, &prevBounce] (size_t tid, size_t startRow, size_t endRow) {

		nanort::Ray<float> ray;
		ray.min_t = RAY_MIN_T;
		ray.max_t = RAY_MAX_T;
		nanort::TriangleIntersector<float> intersector(
			pimpl_->mesh.vertices_, pimpl_->mesh.faces_, sizeof(vec3));

		for(size_t r = startRow; r < endRow; ++r) {
			for(size_t c = 0; c < width_; ++c) {
				vec4 worldPos = worldPosTex_[r*width_ + c];
				if(worldPos.w() == 0) {
					continue;
				}
				vec3 norm = normalTex_[r*width_ + c].block<3,1>(0,0);
				memcpy(ray.org, worldPos.data(), sizeof(vec3));
				std::vector<KahanVal<vec3> > coefftSums;
				for (size_t j = 0; j < calcNumSHCoeffts(nBands_); ++j) {
					coefftSums.emplace_back(vec3::Zero());
				}

				for(size_t i = 0; i < sampleDirs_.size(); ++i) {
					float dotProd = sampleDirs_[i].dot(norm);
					if(dotProd <= 0.f) {
						continue;
					}
					memcpy(ray.dir, sampleDirs_[i].data(), sizeof(vec3));
					nanort::TriangleIntersection<> intersection;
					if(pimpl_->accel.Traverse(ray, intersector, &intersection)){
						vec2 intersectTexCoord =
							(1.f - intersection.u - intersection.v) * 
							pimpl_->texCoords[pimpl_->mesh.faces_[intersection.prim_id * 3 + 0]] +
							intersection.u * 
							pimpl_->texCoords[pimpl_->mesh.faces_[intersection.prim_id * 3 + 1]] +
							intersection.v * 
							pimpl_->texCoords[pimpl_->mesh.faces_[intersection.prim_id * 3 + 2]];
						std::vector<vec3> intersectionCoeffts = 
							samplePrtCoeffts(prevBounce, intersectTexCoord);
						for (size_t j = 0; j < coefftSums.size(); ++j) {
							coefftSums[j] += dotProd * intersectionCoeffts[j];
						}
					}
				}
				vec3 albedo = albedoTex[r*width_ + c];

				for (size_t j = 0; j < coefftSums.size(); ++j) {
					(*out)[j][r*width_ + c] += 
						(albedo.array() * coefftSums[j].get().array()).matrix() / 
						float(sampleDirs_.size()); 
						//Coefft-wise multiply
				}
			}
		}

	});
}

void PrtBaker::bakeDiffuseShadowedPrtSpecular(
	PrtCoeffts<vec3> *out, vec3 *albedoTex)
{
	if (!pimpl_->specularMesh) {
		throw std::runtime_error("Please call setupSpecularMesh before this function!");
	}
	
	parallelProcessImage(width_, height_,
	[this, out, albedoTex] (size_t tid, size_t startRow, size_t endRow) {
		
	nanort::Ray<float> ray;
	nanort::TriangleIntersector<float> intersector(
		pimpl_->mesh.vertices_, pimpl_->mesh.faces_, sizeof(vec3));
	nanort::TriangleIntersector<float> specularIntersector(
		pimpl_->specularMesh->mesh.vertices_, 
		pimpl_->specularMesh->mesh.faces_, sizeof(vec3));
	ray.min_t = RAY_MIN_T;
	ray.max_t = RAY_MAX_T;
		
	for(size_t r = startRow; r < endRow; ++r) {
		for(size_t c = 0; c < width_; ++c) {
			vec4 worldPos = worldPosTex_[r*width_ + c];
			if(worldPos.w() == 0) {
				continue;
			}
			vec3 norm = normalTex_[r*width_ + c].block<3,1>(0,0).normalized();
			memcpy(ray.org, worldPos.data(), sizeof(vec3));
			std::vector<KahanVal<vec3> > coefftSums;
			for (size_t j = 0; j < calcNumSHCoeffts(nBands_); ++j) {
				coefftSums.emplace_back(vec3::Zero());
			}
			 
			for(size_t i = 0; i < sampleDirs_.size(); ++i) {
				float dotProd = sampleDirs_[i].dot(norm);
				if(dotProd <= 0.f) {
					continue;
				}
				memcpy(ray.org, worldPos.data(), sizeof(vec3));
				memcpy(ray.dir, sampleDirs_[i].data(), sizeof(vec3));
				nanort::TriangleIntersection<> intersection;
				nanort::TriangleIntersection<> specularIntersection;
				bool hitDiffuse = pimpl_->accel.Traverse(ray, intersector, &intersection);
				bool hitSpecular = pimpl_->specularMesh->accel.Traverse(ray, specularIntersector, &specularIntersection);
				size_t nSpecularRays = 0;
				while (hitSpecular && (!hitDiffuse || (specularIntersection.t < intersection.t))) {
					//Hit specular object first. Reflect ray in specular object.
					vec3 rayOrg(ray.org[0], ray.org[1], ray.org[2]);
					vec3 rayDir(ray.dir[0], ray.dir[1], ray.dir[2]);

					size_t faceOffset = specularIntersection.prim_id * 3;
					vec3 vert0, vert1, vert2;
					memcpy(&vert0, pimpl_->specularMesh->mesh.vertices_ + pimpl_->specularMesh->mesh.faces_[faceOffset + 0], sizeof(vec3));
					memcpy(&vert1, pimpl_->specularMesh->mesh.vertices_ + pimpl_->specularMesh->mesh.faces_[faceOffset + 1], sizeof(vec3));
					memcpy(&vert2, pimpl_->specularMesh->mesh.vertices_ + pimpl_->specularMesh->mesh.faces_[faceOffset + 2], sizeof(vec3));

					vec3 intersectPos =
						(1.f - specularIntersection.u - specularIntersection.v) * vert0 +
						specularIntersection.u * vert1 +
						specularIntersection.v * vert2;

					vec3 newOrg = intersectPos;

					//Find norm at intersection.
					vec3 intersectNorm =
						(1.f - specularIntersection.u - specularIntersection.v) *
						pimpl_->specularMesh->norms[pimpl_->specularMesh->mesh.faces_[faceOffset + 0]] +
						specularIntersection.u *
						pimpl_->specularMesh->norms[pimpl_->specularMesh->mesh.faces_[faceOffset + 1]] +
						specularIntersection.v *
						pimpl_->specularMesh->norms[pimpl_->specularMesh->mesh.faces_[faceOffset + 2]];
					intersectNorm.normalize();

					//Reflect ray dir.
					vec3 newDir = reflect(rayDir, intersectNorm);
					memcpy(ray.dir, &newDir[0], sizeof(vec3));
					memcpy(ray.org, &newOrg[0], sizeof(vec3));

					//Cast diffuse, spec rays with new dir.				
					hitDiffuse = pimpl_->accel.Traverse(ray, intersector, &intersection);
					hitSpecular = pimpl_->specularMesh->accel.Traverse(ray, specularIntersector, &specularIntersection);
					++nSpecularRays;
					if (nSpecularRays > 5) {
						break;
					}
				} 
				 
				if (!hitDiffuse) {
					for (size_t j = 0; j < coefftSums.size(); ++j) {
						//TODO Think about if thsi is really correct - if the ray was reflected, maybe dir should change here?
						coefftSums[j] += vec3::Ones() * dotProd * sampleShVals_[i][j];
					 }
				}			 
			}
			 vec3 albedo = albedoTex[r*width_ + c];
			 
			 for (size_t j = 0; j < coefftSums.size(); ++j) {
				 (*out)[j][r*width_ + c] = 4.f * M_PI *
				 (albedo.array() * coefftSums[j].get().array()).matrix() / sampleDirs_.size();
				 //Coefft-wise multiply
			 }
		 }
	 }
		
	});
}

void PrtBaker::bakeDiffuseInterreflectedPrtSpecular(
	size_t nBounces, PrtCoeffts<vec3> *out, vec3 *albedoTex)
{
	bakeDiffuseShadowedPrtSpecular(out, albedoTex);
	for(size_t i = 1; i <= nBounces; ++i) {
		std::cout << "Starting bounce " << i << std::endl;
		performInterreflectionPassSpecular(out, albedoTex);
		std::cout << "Bounce " << i << " completed." << std::endl;
	}
}

void PrtBaker::performInterreflectionPassSpecular(
	PrtCoeffts<vec3> *out, vec3 *albedoTex)
{
	if (!pimpl_->specularMesh) {
		throw std::runtime_error("Please call setupSpecularMesh before this function!");
	}
	PrtCoeffts<vec3> prevBounce(*out);
	
	parallelProcessImage(width_, height_,
	[this, out, albedoTex, &prevBounce] (size_t tid, size_t startRow, size_t endRow) {
		 
		nanort::Ray<float> ray;
		ray.min_t = RAY_MIN_T;
		ray.max_t = RAY_MAX_T;
		nanort::TriangleIntersector<float> intersector(
			pimpl_->mesh.vertices_, pimpl_->mesh.faces_, sizeof(vec3));
		nanort::TriangleIntersector<float> specularIntersector(
			pimpl_->specularMesh->mesh.vertices_, 
			pimpl_->specularMesh->mesh.faces_, sizeof(vec3));
		 
		for(size_t r = startRow; r < endRow; ++r) {
			for(size_t c = 0; c < width_; ++c) {
				vec4 worldPos = worldPosTex_[r*width_ + c];
				if(worldPos.w() == 0) {
					 continue;
				}
				vec3 norm = normalTex_[r*width_ + c].block<3,1>(0,0);
				memcpy(ray.org, worldPos.data(), sizeof(vec3));
				std::vector<KahanVal<vec3> > coefftSums;
				for (size_t j = 0; j < calcNumSHCoeffts(nBands_); ++j) {
					 coefftSums.emplace_back(vec3::Zero());
				}
				 
				for(size_t i = 0; i < sampleDirs_.size(); ++i) {
					float dotProd = sampleDirs_[i].dot(norm);
					if(dotProd <= 0.f) {
						continue;
					}
					memcpy(ray.org, worldPos.data(), sizeof(vec3));
					memcpy(ray.dir, sampleDirs_[i].data(), sizeof(vec3));

					nanort::TriangleIntersection<> intersection;
					nanort::TriangleIntersection<> specularIntersection;
					bool hitDiffuse = pimpl_->accel.Traverse(ray, intersector, &intersection);
					bool hitSpecular = pimpl_->specularMesh->accel.Traverse(ray, specularIntersector, &specularIntersection);
					size_t nSpecularRays = 0;
					while (hitSpecular && (!hitDiffuse || (specularIntersection.t < intersection.t))) {
						//Hit specular object first. Reflect ray in specular object.
						vec3 rayOrg(ray.org[0], ray.org[1], ray.org[2]);
						vec3 rayDir(ray.dir[0], ray.dir[1], ray.dir[2]);
						size_t faceOffset = specularIntersection.prim_id * 3;
						vec3 vert0, vert1, vert2;
						memcpy(&vert0, pimpl_->specularMesh->mesh.vertices_ + pimpl_->specularMesh->mesh.faces_[faceOffset + 0], sizeof(vec3));
						memcpy(&vert1, pimpl_->specularMesh->mesh.vertices_ + pimpl_->specularMesh->mesh.faces_[faceOffset + 1], sizeof(vec3));
						memcpy(&vert2, pimpl_->specularMesh->mesh.vertices_ + pimpl_->specularMesh->mesh.faces_[faceOffset + 2], sizeof(vec3));

						vec3 intersectPos =
							(1.f - specularIntersection.u - specularIntersection.v) * vert0 +
							specularIntersection.u * vert1 +
							specularIntersection.v * vert2;

						vec3 newOrg = intersectPos;

						//Find norm at intersection.
						vec3 intersectNorm =
							(1.f - specularIntersection.u - specularIntersection.v) *
							pimpl_->specularMesh->norms[pimpl_->specularMesh->mesh.faces_[faceOffset + 0]] +
							specularIntersection.u *
							pimpl_->specularMesh->norms[pimpl_->specularMesh->mesh.faces_[faceOffset + 1]] +
							specularIntersection.v *
							pimpl_->specularMesh->norms[pimpl_->specularMesh->mesh.faces_[faceOffset + 2]];
						intersectNorm.normalize();

						//Reflect ray dir.
						vec3 newDir = reflect(rayDir, intersectNorm);
						memcpy(ray.dir, &newDir[0], sizeof(vec3));
						memcpy(ray.org, &newOrg[0], sizeof(vec3));

						//Cast diffuse, spec rays with new dir.				
						hitDiffuse = pimpl_->accel.Traverse(ray, intersector, &intersection);
						hitSpecular = pimpl_->specularMesh->accel.Traverse(ray, specularIntersector, &specularIntersection);
						++nSpecularRays;
						if (nSpecularRays > 5) {
							break;
						}
					} 
					if(hitDiffuse){
						vec2 intersectTexCoord =
							(1.f - intersection.u - intersection.v) *
							pimpl_->texCoords[pimpl_->mesh.faces_[intersection.prim_id * 3 + 0]] +
							intersection.u *
							pimpl_->texCoords[pimpl_->mesh.faces_[intersection.prim_id * 3 + 1]] +
							intersection.v *
							pimpl_->texCoords[pimpl_->mesh.faces_[intersection.prim_id * 3 + 2]];
						std::vector<vec3> intersectionCoeffts =
							samplePrtCoeffts(prevBounce, intersectTexCoord);
						for (size_t j = 0; j < coefftSums.size(); ++j) {
							coefftSums[j] += dotProd * intersectionCoeffts[j];
						}
					 }
				}
				vec3 albedo = albedoTex[r*width_ + c];
				 
				for (size_t j = 0; j < coefftSums.size(); ++j) {
					(*out)[j][r*width_ + c] +=
					(albedo.array() * coefftSums[j].get().array()).matrix() /
					float(sampleDirs_.size());
					//Coefft-wise multiply
				}
			}
		}
	});
}

}

