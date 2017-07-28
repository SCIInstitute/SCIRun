#ifndef IAUNS_GP_REN_UTIL_TYPE_TO_GL_UNIFORM_HPP
#define IAUNS_GP_REN_UTIL_TYPE_TO_GL_UNIFORM_HPP

#include <gl-platform/GLPlatform.hpp>
#include "es-render/comp/VecUniform.hpp"

namespace ren {

namespace uniform_detail {

template <typename T>
class TypeToGLUniform
{
public:
  typedef int Type;

  static_assert(true, "TypeToGLUniform type specialization not defined.");
  static void addUniform(CPM_ES_CEREAL_NS::CerealCore& core, uint64_t entityID,
                         const char* uniformName, const Type& v);
  //static void setUniform(Core& core, uint64_t entityId, const char* uniformName, const Type& v);
};

template<>
class TypeToGLUniform<float>
{
public:
  typedef float Type;

  static void addUniform(CPM_ES_CEREAL_NS::CerealCore& core, uint64_t entityID, const char* uniformName, const Type& v)
  {
    ren::VecUniform unif(uniformName, glm::vec4(v, 0.0f, 0.0f, 0.0f), ren::VecUniform::SCALAR);
    core.addComponent(entityID, unif);
  }
};

template<>
class TypeToGLUniform<glm::vec2>
{
public:
  typedef glm::vec2 Type;

  static void addUniform(CPM_ES_CEREAL_NS::CerealCore& core, uint64_t entityID, const char* uniformName, const Type& v)
  {
    ren::VecUniform unif(uniformName, glm::vec4(v.x, v.y, 0.0f, 0.0f), ren::VecUniform::VEC2);
    core.addComponent(entityID, unif);
  }
};

template<>
class TypeToGLUniform<glm::vec3>
{
public:
  typedef glm::vec3 Type;

  static void addUniform(CPM_ES_CEREAL_NS::CerealCore& core, uint64_t entityID, const char* uniformName, const Type& v)
  {
    ren::VecUniform unif(uniformName, glm::vec4(v.x, v.y, v.z, 0.0f), ren::VecUniform::VEC3);
    core.addComponent(entityID, unif);
  }
};

template<>
class TypeToGLUniform<glm::vec4>
{
public:
  typedef glm::vec4 Type;

  static void addUniform(CPM_ES_CEREAL_NS::CerealCore& core, uint64_t entityID, const char* uniformName, const Type& v)
  {
    ren::VecUniform unif(uniformName, glm::vec4(v.x, v.y, v.z, v.w), ren::VecUniform::VEC4);
    core.addComponent(entityID, unif);
  }
};

} // namespace uniform_detail

} // namespace ren

#endif
