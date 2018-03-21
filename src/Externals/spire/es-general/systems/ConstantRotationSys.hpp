#ifndef SPIRE_ES_GENERAL_SYSTEMS_CONSTANT_ROTATION_HPP
#define SPIRE_ES_GENERAL_SYSTEMS_CONSTANT_ROTATION_HPP

#include <es-log/trace-log.h>
#include <es-acorn/Acorn.hpp>
#include <spire/scishare.h>

namespace gen {

void        registerSystem_ConstantRotation(spire::Acorn& core);
const char* getSystemName_ConstantRotation();

} // namespace gen

#endif
