#ifndef IAUNS_RENDER_SIMPLE_GEOM_SYS_HPP
#define IAUNS_RENDER_SIMPLE_GEOM_SYS_HPP

#include <es-acorn/Acorn.hpp>

namespace ren {

void        registerSystem_RenderSimpleGeom(CPM_ES_ACORN_NS::Acorn& core);
const char* getSystemName_RenderSimpleGeom();

} // namespace ren

#endif
