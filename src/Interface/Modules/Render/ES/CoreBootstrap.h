#ifndef SCIRUN_RENDER_ES_COREBOOTSTRAP_HPP
#define SCIRUN_RENDER_ES_COREBOOTSTRAP_HPP

#include <es-acorn/Acorn.hpp>

// Common bootstrap for the entire game application.
namespace SCIRun {
namespace Render {

// Two required functions for systems. Technically, systems could
// register themselves.
void        registerSystem_CoreBootstrap(CPM_ES_ACORN_NS::Acorn& core);
const char* getSystemName_CoreBootstrap();

} // namespace Render
} // namespace SCIRun

#endif 
