#define _USE_MATH_DEFINES
#include "glue/Raycast.hpp"
#include <Eigen/Dense>
#include <thread>
#include <fstream>
#include <iostream>
#include <cfloat>

namespace glue {

const float EPS = 10e-6f;
vec3b BACKGROUND_COLOR = vec3b(0,0,0);

void raycast(
	vec3b *out,
	const std::vector<vec3> &vertices,
	const std::vector<unsigned int> &indices,
	const std::vector<vec3b> &colors,
	const WorldToCamTransform &worldToCamera,
	const CameraModel &model,
	float *depths)
{
	bool raycastDepths = depths != nullptr;
	glue::WorldToCamTransform inv = worldToCamera.inverse();
	vec4 origin4 = worldToCamera * vec4(0.f, 0.f, 0.f, 1.f);
	vec3 origin(origin4.x(), origin4.y(), origin4.z());
	mat3 invRot = worldToCamera.block<3,3>(0,0).inverse();
	for(size_t r = 0; r < model.h; ++r) {
		for(size_t c = 0; c < model.w; ++c) {
			out[r*model.w + c] = vec3b(255,0,0);
		}
	}

	if (raycastDepths) {
		//Set up depths image if necessary.
		for(size_t r = 0; r < model.h; ++r) {
			for(size_t c = 0; c < model.w; ++c) {
				depths[r*model.w + c] = -1.f;
			}
		}
	}

	std::vector<std::thread> threads(model.h);
	for(size_t r = 0; r < size_t(model.h); ++r) {
		threads[r] = std::thread([
		r, out, &vertices, &indices, &colors,
		&worldToCamera, &model, &origin, &invRot,
		raycastDepths, &depths]() {
			//vec3b *rPtr = image.ptr<vec3b>(r);
			for(size_t c = 0; c < size_t(model.w); ++c) {
    			Ray ray;
    			ray.origin = origin;
    			ray.dir = /*invRot * */model.pixelToCam(vec2(r,c));
				float depth;
				out[r*model.w + c] =
					shootRay(ray, vertices, indices, colors, depth);
				if (raycastDepths) {
					depths[r*model.w + c] = depth;
				}
			}
		});
	}
	
	for(std::thread &t : threads) {
		t.join();
	}
}

std::vector<vec3> modelToPointCloud(
	const std::vector<vec3> &vertices,
	const std::vector<unsigned int> &indices,
	const vec3 &center, size_t desiredPoints,
	float gaussNoiseStd)
{
	size_t maxRaycasts = 5 * desiredPoints;

	std::vector<vec3> pointCloud;
	std::default_random_engine randEngine;
	std::normal_distribution<float> normDist(0.f, gaussNoiseStd != 0.f ? gaussNoiseStd : 1.f);
	for (size_t i = 0; i < maxRaycasts; ++i) {
		Ray ray;
		ray.origin = center;
		ray.dir = randDir(randEngine);
		bool hit;
		vec3 intersection = shootRay(ray, vertices, indices, hit);
		if (hit) {
			if (gaussNoiseStd != 0) {
				intersection += normDist(randEngine)*randDir(randEngine);
			}
			pointCloud.push_back(intersection);
			if (pointCloud.size() == desiredPoints) {
				break;
			}
		}
	}

	if (pointCloud.size() < desiredPoints) {
		std::cout << "Terminating early - " << desiredPoints << " were reques"
			"ted, but only " << pointCloud.size() << " intersections were fo"
			"und in " << maxRaycasts << " attempts." << std::endl;
	}

	return pointCloud;
}

vec3 randDir(std::default_random_engine &device)
{
	std::uniform_real_distribution<float> phiDist(0.f, 1.f);
	std::uniform_real_distribution<float> thetaDist(float(-M_PI), float(M_PI));
	float theta = thetaDist(device);
	float phi = acosf(2.f * phiDist(device) - 1.f);
	return vec3(sin(theta) * sin(phi), cos(theta) * sin(phi), cos(phi));
}

vec3b shootRay(
	const Ray &ray,
	const std::vector<vec3> &vertices,
	const std::vector<unsigned int> &indices,
	const std::vector<vec3b> &colors,
	float &depth)
{
	float closestIntersectionDist = FLT_MAX;
	vec3b color = BACKGROUND_COLOR;
	for(size_t i = 0; i+2 < indices.size(); i += 3) {
		float dist;
		if(intersectRayWithTriangle(
			vertices[indices[i]],
			vertices[indices[i+1]],
			vertices[indices[i+2]],
			ray, dist)) {
				if(dist < closestIntersectionDist) {
					closestIntersectionDist = dist;
					color = colors.at(indices[i]);
				}
		}
		else {
		}
	}
	if (closestIntersectionDist == FLT_MAX) {
		depth = -1.f;
	} else {
		depth = closestIntersectionDist;
	}
	return color;
}

vec3 shootRay(
	const Ray &ray,
	const std::vector<vec3> &vertices,
	const std::vector<unsigned int> &indices, 
	bool &hit)
{
	float closestIntersectionDist = FLT_MAX;
	for(size_t i = 0; i+2 < indices.size(); i += 3) {
		float dist;
		if(intersectRayWithTriangle(
			vertices[indices[i]],
			vertices[indices[i+1]],
			vertices[indices[i+2]],
			ray, dist)) {
				if(dist < closestIntersectionDist) {
					closestIntersectionDist = dist;
				}
		}
		else {
		}
	}
	if (closestIntersectionDist == FLT_MAX) {
		hit = false;
		return vec3::Zero();
	} else {
		hit = true;
		return ray.origin + ray.dir * closestIntersectionDist;
	}
}

bool intersectRayWithTriangle(
	const vec3 &ta,
	const vec3 &tb,
	const vec3 &tc,
	const Ray &ray,
	float &dist)
{
	// Get two edges of the triangle.
	vec3 e1 = tb - ta;
	vec3 e2 = tc - ta;
	
	vec3 norm = (ray.dir).cross(e2);
	
	if (norm.norm() < EPS) {
		return false; //Triangle has no area.
	}
	
	float det = e1.dot(norm);
	
	if(det < EPS && det > -EPS) return false; //Ray lies in plane of triangle.
	
	float oneOverDet = 1.0f / det;
	
	vec3 toTriangle = (ray.origin) - ta;
	// Find barycentric co-ordinates u,v.
	float u = toTriangle.dot(norm) * oneOverDet;
	if(u < 0.0f || u > 1.0f) return false; // u out of range.
	
	vec3 acrossTriangle = toTriangle.cross(e1);
	float v = ray.dir.dot(acrossTriangle) * oneOverDet;
	if(v < 0.0f || u + v > 1.0f) return false; // v out of range.
	
	float t = e2.dot(acrossTriangle) * oneOverDet;
	
	if(t < EPS) return false; // intersection lies behind/on ro.
	
	dist = t;
	
	return true; // u, v in range - intersection.
}

mat4 loadCamTransform(const std::string &filename)
{
	mat4 t;
	std::ifstream i(filename);
	for(int r = 0; r < 4; ++r) {
		for(int c = 0; c < 4; ++c) {
			i >> t(r,c);
		}
	}
	return t;
}

}
