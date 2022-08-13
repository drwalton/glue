#ifndef GLUE_PRTMESHCOLOR_HPP_INCLUDED
#define GLUE_PRTMESHCOLOR_HPP_INCLUDED

#include "glue/PrtMesh.hpp"

namespace glue {

//!\brief Class abstracting a renderable 3D triangle mesh.
class PrtMeshColor : public PrtMesh
{
public:
	//!\brief Constructor for Mono PRT mesh.
	PrtMeshColor(const std::string &meshFilename,
		const std::string &prtCoefftsFilename);

	~PrtMeshColor() throw();
	
private:
	PrtMeshColor(const PrtMeshColor &);
	PrtMeshColor &operator=(const PrtMeshColor &);
};

}

#endif
