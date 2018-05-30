#ifndef SPIRE_ASYNC_REGISTRATION_HPP
#define SPIRE_ASYNC_REGISTRATION_HPP

#include <es-log/trace-log.h>
#include <es-acorn/Acorn.hpp>
#include <spire/scishare.h>

namespace spire {

/// Registers all systems and components.
  SCISHARE void registerAll(spire::Acorn& core);

} // namespace ren

#endif
