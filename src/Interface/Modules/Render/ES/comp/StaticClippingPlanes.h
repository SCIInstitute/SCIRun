#ifndef INTERFACE_MODULES_RENDER_ES_COMP_STATIC_CLIPPING_PLANES_H
#define INTERFACE_MODULES_RENDER_ES_COMP_STATIC_CLIPPING_PLANES_H

#include <cstdint>
#include <glm/glm.hpp>
#include <entity-system/GenericSystem.hpp>
#include <cereal-glm/CerealGLM.hpp>
#include <es-cereal/ComponentSerialize.hpp>

namespace SCIRun {
  namespace Render {

    // Static world light. We may have multiple world lights inside this structure
    // eventually.
    struct StaticClippingPlanes
    {
      // -- Data --
      std::vector<glm::vec4> clippingPlanes;
      std::vector<glm::vec4> clippingPlaneCtrls;

      // -- Functions --
      StaticClippingPlanes() { }

      static const char* getName() { return "StaticClippingPlanes"; }

      bool serialize(CPM_ES_CEREAL_NS::ComponentSerialize& s, uint64_t /* entityID */)
      {
        //s.serialize("clipping_planes", clippingPlanes);
        return true;
      }
    };

  } // namespace Render
} // namespace SCIRun

#endif 
