#ifndef GLUE_RENDERDOC_HPP_INCLUDED
#define GLUE_RENDERDOC_HPP_INCLUDED

#include "glue/NonCopyable.hpp"

namespace glue {

class RenderDoc final : NonCopyable
{
public:
	explicit RenderDoc();
	~RenderDoc() throw();

	void triggerCapture();

private:
	struct Impl;
	Impl *pimpl_;
};

}

#endif
