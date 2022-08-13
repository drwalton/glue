#include "glue/AxisHelper.hpp"
#include <array>

const std::array<GLuint, 36> cuboidElems = {
	0, 1, 2,
	0, 2, 3,

	3, 2, 6,
	3, 6, 7,

	0, 3, 7,
	0, 7, 4,

	1, 0, 4,
	1, 4, 5,

	2, 1, 5,
	2, 5, 6,

	5, 4, 7,
	5, 7, 6
};

namespace glue {

AxisHelper::AxisHelper(float length, float diameter)
{
	std::vector<vec3> verts, colors;
	std::vector<GLuint> elems;

	//x-axis (red)
	verts.push_back(vec3(0, -diameter, -diameter));
	verts.push_back(vec3(0, -diameter, diameter));
	verts.push_back(vec3(0, diameter, diameter));
	verts.push_back(vec3(0, diameter, -diameter));
	verts.push_back(vec3(length, -diameter, -diameter));
	verts.push_back(vec3(length, -diameter, diameter));
	verts.push_back(vec3(length, diameter, diameter));
	verts.push_back(vec3(length, diameter, -diameter));
	colors.push_back(vec3(1, 0, 0));
	colors.push_back(vec3(1, 0, 0));
	colors.push_back(vec3(1, 0, 0));
	colors.push_back(vec3(1, 0, 0));
	colors.push_back(vec3(1, 0, 0));
	colors.push_back(vec3(1, 0, 0));
	colors.push_back(vec3(1, 0, 0));
	colors.push_back(vec3(1, 0, 0));
	for (size_t i = 0; i < cuboidElems.size(); ++i) {
		elems.push_back(cuboidElems[i]);
	}

	//y-axis (green)
	verts.push_back(vec3(-diameter, 0, -diameter));
	verts.push_back(vec3(diameter, 0, -diameter));
	verts.push_back(vec3(diameter, 0, diameter));
	verts.push_back(vec3(-diameter, 0, diameter));
	verts.push_back(vec3(-diameter, length, -diameter));
	verts.push_back(vec3(diameter, length, -diameter));
	verts.push_back(vec3(diameter, length, diameter));
	verts.push_back(vec3(-diameter, length, diameter));
	colors.push_back(vec3(0, 1, 0));
	colors.push_back(vec3(0, 1, 0));
	colors.push_back(vec3(0, 1, 0));
	colors.push_back(vec3(0, 1, 0));
	colors.push_back(vec3(0, 1, 0));
	colors.push_back(vec3(0, 1, 0));
	colors.push_back(vec3(0, 1, 0));
	colors.push_back(vec3(0, 1, 0));
	for (size_t i = 0; i < cuboidElems.size(); ++i) {
		elems.push_back(cuboidElems[i] + 8);
	}

	//z-axis (blue)
	verts.push_back(vec3(diameter, -diameter, 0));
	verts.push_back(vec3(-diameter, -diameter, 0));
	verts.push_back(vec3(-diameter, diameter, 0));
	verts.push_back(vec3(diameter, diameter, 0));
	verts.push_back(vec3(diameter, -diameter, length));
	verts.push_back(vec3(-diameter, -diameter, length));
	verts.push_back(vec3(-diameter, diameter, length));
	verts.push_back(vec3(diameter, diameter, length));
	colors.push_back(vec3(0, 0, 1));
	colors.push_back(vec3(0, 0, 1));
	colors.push_back(vec3(0, 0, 1));
	colors.push_back(vec3(0, 0, 1));
	colors.push_back(vec3(0, 0, 1));
	colors.push_back(vec3(0, 0, 1));
	colors.push_back(vec3(0, 0, 1));
	colors.push_back(vec3(0, 0, 1));
	for (size_t i = 0; i < cuboidElems.size(); ++i) {
		elems.push_back(cuboidElems[i] + 16);
	}

	color(colors);
	vert(verts);
	this->elems(elems);
}

AxisHelper::~AxisHelper() throw()
{

}

}
