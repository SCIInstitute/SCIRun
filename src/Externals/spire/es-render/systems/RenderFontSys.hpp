#ifndef SPIRE_RENDER_RENDERFONTSYS_HPP
#define SPIRE_RENDER_RENDERFONTSYS_HPP

#include <es-log/trace-log.h>
#include <es-acorn/Acorn.hpp>

namespace ren {

void        registerSystem_RenderFont(spire::Acorn& core);
const char* getSystemName_RenderFont();

} // namespace ren

#endif
