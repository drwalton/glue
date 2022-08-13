#ifndef GLUE_PRTMESHCOLORSHADOWED_HPP_INCLUDED
#define GLUE_PRTMESHCOLORSHADOWED_HPP_INCLUDED

#include "glue/PrtMesh.hpp"

namespace glue {

//!\brief Class abstracting a renderable 3D triangle mesh.
class PrtMeshColorShadowed : public PrtMesh
{
public:
	//!\brief Constructor for Mono PRT mesh.
	PrtMeshColorShadowed(const std::string &meshFilename,
		const std::string &prtCoefftsFilename,
		const std::string &shadowMeshFilename);

	~PrtMeshColorShadowed() throw();
	
private:
	PrtMeshColorShadowed(const PrtMeshColorShadowed &);
	PrtMeshColorShadowed &operator=(const PrtMeshColorShadowed &);
};

}

#endif
