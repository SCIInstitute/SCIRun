#ifndef SPIRE_RENDER_SYSTEMS_DEBUG_CLICKBOX2D_HPP
#define SPIRE_RENDER_SYSTEMS_DEBUG_CLICKBOX2D_HPP

#include <es-log/trace-log.h>
#include <es-acorn/Acorn.hpp>

namespace ren {

void        registerSystem_DebugRenderClickBox2D(spire::Acorn& core);
const char* getSystemName_DebugRenderClickBox2D();

} // namespace ren

#endif
