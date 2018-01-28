#ifndef SPIRE_RENDER_SIMPLE_GEOM_SYS_HPP
#define SPIRE_RENDER_SIMPLE_GEOM_SYS_HPP

#include <es-log/trace-log.h>
#include <es-acorn/Acorn.hpp>
#include <spire/scishare.h>

namespace ren {

void        registerSystem_RenderSimpleGeom(spire::Acorn& core);
const char* getSystemName_RenderSimpleGeom();

} // namespace ren

#endif
