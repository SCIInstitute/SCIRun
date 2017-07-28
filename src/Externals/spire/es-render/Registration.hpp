#ifndef IAUNS_REN_REGISTRATION_HPP
#define IAUNS_REN_REGISTRATION_HPP

#include <es-acorn/Acorn.hpp>

namespace ren {

/// Registers all gameplay systems and components.
void registerAll(CPM_ES_ACORN_NS::Acorn& core);

/// Functions split up due to file too big to compile in Windows/debug mode.
void register1(CPM_ES_ACORN_NS::Acorn& core);
void register2(CPM_ES_ACORN_NS::Acorn& core);
void register3(CPM_ES_ACORN_NS::Acorn& core);

} // namespace ren

#endif 
