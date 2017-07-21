#ifndef IAUNS_ES_GENERAL_SYSTEMS_CONSTANT_ROTATION_HPP
#define IAUNS_ES_GENERAL_SYSTEMS_CONSTANT_ROTATION_HPP

#include <es-acorn/Acorn.hpp>

namespace gen {

void        registerSystem_ConstantRotation(CPM_ES_ACORN_NS::Acorn& core);
const char* getSystemName_ConstantRotation();

} // namespace gen

#endif 
