#ifndef GLUE_RAYCAST_HPP_INCLUDED
#define GLUE_RAYCAST_HPP_INCLUDED

#include "CameraModel.hpp"
#include <random>

namespace glue {

//!\brief Ray object used by functions in Raycast.hpp
struct Ray {
	vec3 origin;
	vec3 dir;
};

//!\brief Generate a raycasted image given a triangle mesh and a camera model.
//!       Optionally also generate depth maps.
//!\param vertices Vertices of the triangle mesh.
//!\param indices Indices of the triangles (in same format as GL_TRIANGLES)
//!\param colors Vertex colours of the triangles (N.B. only the one with the
//!              index occuring first will be used - try to ensure that the
//!              colours at all vertices of a triangle are the same.
//!\param worldToCamera Transform from world space (space of mesh) to camera
//!                     space (image centre).
//!\param model The camera model used to generate the images.
//!\param[out] depths Optional 32-bit float output containing depth map. Pixels with
//!                   no intersection have depth -1.f. If set to nullptr, will not
//!                   be used.
void raycast(
	vec3b *outIm,
	const std::vector<vec3> &vertices,
	const std::vector<unsigned int> &indices,
	const std::vector<vec3b> &colors,
	const WorldToCamTransform &worldToCamera,
	const CameraModel &model,
	float *depths = nullptr);

//!\brief Generate a cloud of 3D points by raycasting against a triangle mesh.
std::vector<vec3> modelToPointCloud(
	const std::vector<vec3> &vertices,
	const std::vector<unsigned int> &indices,
	const vec3 &center, size_t desiredPoints,
	float gaussNoiseStd = 0.f);

//!\brief Generate a random vector of length 1.
//!\note Uses sampling which is uniform over the unit sphere.
vec3 randDir(std::default_random_engine &device);

vec3b shootRay(
	const Ray &ray,
	const std::vector<vec3> &vertices,
	const std::vector<unsigned int> &indices,
	const std::vector<vec3b> &colors,
	float &depth);

vec3 shootRay(
	const Ray &ray,
	const std::vector<vec3> &vertices,
	const std::vector<unsigned int> &indices,
	bool &hit);

bool intersectRayWithTriangle(
	const vec3 &ta,
	const vec3 &tb,
	const vec3 &tc,
	const Ray &ray,
	float &dist);

mat4 loadCamTransform(const std::string &filename);

}

#endif
