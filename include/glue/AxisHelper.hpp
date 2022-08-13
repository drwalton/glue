#ifndef GLUE_AXISHELPER_HPP_INCLUDED
#define GLUE_AXISHELPER_HPP_INCLUDED

#include "Mesh.hpp"

namespace glue
{

class AxisHelper final : public Mesh {
public:
	AxisHelper(float length, float diameter);
	~AxisHelper() throw();
};

}

#endif
