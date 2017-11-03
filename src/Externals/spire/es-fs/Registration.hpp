#ifndef SPIRE_ASYNC_REGISTRATION_HPP
#define SPIRE_ASYNC_REGISTRATION_HPP

#include <es-log/trace-log.h>
#include <es-acorn/Acorn.hpp>

namespace spire {

/// Registers all gameplay systems and components.
void registerAll(spire::Acorn& core);

} // namespace ren

#endif
