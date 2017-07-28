#ifndef IAUNS_RENDER_RENDERFONTSYS_HPP
#define IAUNS_RENDER_RENDERFONTSYS_HPP

#include <es-acorn/Acorn.hpp>

namespace ren {

void        registerSystem_RenderFont(CPM_ES_ACORN_NS::Acorn& core);
const char* getSystemName_RenderFont();

} // namespace ren

#endif 
