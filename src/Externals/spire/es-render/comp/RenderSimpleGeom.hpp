#ifndef IAUNS_RENDER_COMPONENT_RENDER_SIMPLE_GEOM_HPP
#define IAUNS_RENDER_COMPONENT_RENDER_SIMPLE_GEOM_HPP

#include <gl-shaders/GLShader.hpp>
#include <es-cereal/ComponentSerialize.hpp>

#include "StaticVBOMan.hpp"

namespace ren {

/// \todo Transition this class to use the template ShaderVBOAttribs class
///       under utils (utils/Shader.hpp).
struct RenderSimpleGeom
{
  // -- Data --
  static const int MaxNumAttributes = 5;
  
  int attribSize;     ///< How many elements in 'appliedAttribs' are valid.
  size_t stride;      ///< Stride between elements vertex buffer.
  CPM_GL_SHADERS_NS::ShaderAttributeApplied appliedAttribs[MaxNumAttributes]; 

  // -- Functions --

  RenderSimpleGeom()
  {
    attribSize = -1;
  }

  static const char* getName() {return "ren:RenderSimpleGeom";}

  // Returns true if this simple geom instance has been appropriately setup
  // to handle rendering.
  bool isSetUp() const;

  void setAsUninitialized()
  {
    attribSize = -1;
  }

  // Constructs attribute array given the VBOMan, vbo glid, and shader glid.
  // You will need to const-cast this component as this function modifies
  // the component in-place.
  void checkAttribArray(GLuint vboID, GLuint shaderID,
                        const StaticVBOMan& vboMan);

  bool serialize(CPM_ES_CEREAL_NS::ComponentSerialize& /* s */, uint64_t /* entityID */)
  {
    // Shouldn't need to serialize these values. They are context specific.
    // Maybe? Will need to figure out as I go along.
    return true;
  }
};

} // namespace ren

#endif 
