#include "glue/RenderDoc.hpp"

#ifdef _WIN32
#include <windows.h>
#include <renderdoc_app.h>


namespace glue {

struct RenderDoc::Impl {
	HMODULE module;
	pRENDERDOC_GetAPI RENDERDOC_GetAPI;
	RENDERDOC_API_1_1_1 *api;
	bool valid;
};

RenderDoc::RenderDoc()
{
	pimpl_ = new Impl;
	pimpl_->valid = false;
	LPCSTR moduleName = { "renderdoc.dll" };
	pimpl_->module = GetModuleHandle(moduleName);
	if (pimpl_->module) {
		LPCSTR procName = { "RENDERDOC_GetAPI" };
		pimpl_->RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)GetProcAddress(pimpl_->module, procName);
		if (pimpl_->RENDERDOC_GetAPI) {
			if (pimpl_->RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_1, (void**)(&pimpl_->api))) {
				pimpl_->valid = true;
			}
		}
	}
}

RenderDoc::~RenderDoc() throw()
{
	delete pimpl_;
}

void RenderDoc::triggerCapture()
{
	if (pimpl_->valid) {
		pimpl_->api->TriggerCapture();
	}
}

}

//TODO Linux implementation(?)
#endif
