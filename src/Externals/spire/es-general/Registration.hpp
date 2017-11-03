#ifndef SPIRE_ES_GENERAL_REGISTRATION_HPP
#define SPIRE_ES_GENERAL_REGISTRATION_HPP

#include <es-log/trace-log.h>
#include <es-acorn/Acorn.hpp>

namespace gen {

/// Registers all gameplay systems and components.
void registerAll(spire::Acorn& core);

} // namespace gen

#endif
