#ifndef SPIRE_ES_GENERAL_REGISTRATION_HPP
#define SPIRE_ES_GENERAL_REGISTRATION_HPP

#include <es-log/trace-log.h>
#include <es-acorn/Acorn.hpp>
#include <spire/scishare.h>

namespace gen {

/// Registers all systems and components.
  SCISHARE void registerAll(spire::Acorn& core);

} // namespace gen

#endif
