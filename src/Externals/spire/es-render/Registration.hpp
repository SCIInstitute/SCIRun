#ifndef SPIRE_REN_REGISTRATION_HPP
#define SPIRE_REN_REGISTRATION_HPP

#include <es-log/trace-log.h>
#include <es-acorn/Acorn.hpp>

namespace ren {

/// Registers all gameplay systems and components.
void registerAll(spire::Acorn& core);

/// Functions split up due to file too big to compile in Windows/debug mode.
void register1(spire::Acorn& core);
void register2(spire::Acorn& core);
void register3(spire::Acorn& core);

} // namespace ren

#endif
