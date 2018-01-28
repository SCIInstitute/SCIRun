#ifndef SPIRE_GAMEPLAY_SYSTEMS_UTIL_VIEW_POS_ALIGN_HPP
#define SPIRE_GAMEPLAY_SYSTEMS_UTIL_VIEW_POS_ALIGN_HPP

#include <es-log/trace-log.h>
#include <es-acorn/Acorn.hpp>
#include <spire/scishare.h>

namespace ren {

void        registerSystem_UtilViewPosAlign(spire::Acorn& core);
const char* getSystemName_UtilViewPosAlign();

} // namespace ren

#endif
