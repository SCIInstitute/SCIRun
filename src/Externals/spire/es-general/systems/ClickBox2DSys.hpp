#ifndef SPIRE_ES_GENERAL_SYSTEMS_CLICKBOX2D_HPP
#define SPIRE_ES_GENERAL_SYSTEMS_CLICKBOX2D_HPP

#include <es-log/trace-log.h>
#include <es-acorn/Acorn.hpp>
#include <spire/scishare.h>

namespace gen {

void        registerSystem_ClickBox2D(spire::Acorn& core);
const char* getSystemName_ClickBox2D();

} // namespace gen

#endif
