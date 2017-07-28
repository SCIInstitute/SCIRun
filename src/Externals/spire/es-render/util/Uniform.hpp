#ifndef IAUNS_GP_REN_UTIL_UNIFORM_HPP
#define IAUNS_GP_REN_UTIL_UNIFORM_HPP

#include <es-cereal/CerealCore.hpp>
#include "helper/TypeToGLUniform.hpp"

namespace ren {

// This function *only* adds a uniform. It does not check for pre-existing
// uniforms. Faster and simpler if we know the uniform doesn't already
// exist (such as when the object is first being created).
template <typename T>
void addGLUniform(CPM_ES_CEREAL_NS::CerealCore& core, uint64_t entityID,
                  const char* uniformName, const T& val)
{
  uniform_detail::TypeToGLUniform<T>::addUniform(core, entityID, uniformName, val);
}

// Detects appropriate uniform to add to the entity. Will attempt to find
// a pre-existing uniform in its component list and modify it instead of 
// adding a new one. Not implemented yet, check the other ren-utils that
// check components. For this implementation, iteration over the components
// will be necessary. May need to add functionality for this.
//template <typename T>
//void setGLUniform(CPM_ES_CEREAL_NS::CerealCore& core, uint64_t entityID, const T& val) ...

} // namespace ren

#endif 
