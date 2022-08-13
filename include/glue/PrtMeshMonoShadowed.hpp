#ifndef GLUE_PRTMESHMONOSHADOWED_HPP_INCLUDED
#define GLUE_PRTMESHMONOSHADOWED_HPP_INCLUDED

#include "glue/PrtMesh.hpp"

namespace glue {

//!\brief Class abstracting a renderable 3D triangle mesh.
class PrtMeshMonoShadowed : public PrtMesh
{
public:
	//!\brief Constructor for Mono PRT mesh.
	PrtMeshMonoShadowed(
		const std::string & meshFilename, 
		const std::string & prtCoefftsFilename,
		const std::string &shadowMeshFilename);

	~PrtMeshMonoShadowed() throw();
	
private:
	PrtMeshMonoShadowed(const PrtMeshMonoShadowed &);
	PrtMeshMonoShadowed &operator=(const PrtMeshMonoShadowed &);
};

}

#endif
