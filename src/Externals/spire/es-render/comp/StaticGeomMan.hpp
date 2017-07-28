#ifndef IAUNS_RENDER_COMPONENT_STATIC_GEOM_MAN_HPP
#define IAUNS_RENDER_COMPONENT_STATIC_GEOM_MAN_HPP

#include <memory>
#include <es-cereal/ComponentSerialize.hpp>
#include "../GeomMan.hpp"

namespace ren {

struct StaticGeomMan
{
    // -- Data --
    std::shared_ptr<GeomMan> instance_;

    // -- Functions --
    StaticGeomMan() : instance_(std::shared_ptr<GeomMan>(new GeomMan)) {}
    StaticGeomMan(GeomMan* s) : instance_(std::shared_ptr<GeomMan>(s)) {}

    // This assignment operator is only used during modification calls inside
    // of the entity system. We don't care about those calls as they won't
    // affect this static shader man.
    StaticGeomMan& operator=(const StaticGeomMan&)
    {
    // We don't care about the incoming object. We've already created oun own
    // shader man and will continue to use that.
    return *this;
    }

    static const char* getName() {return "ren:StaticGeomMan";}

private:
    friend class CPM_ES_CEREAL_NS::CerealHeap<StaticGeomMan>;

    bool serialize(CPM_ES_CEREAL_NS::ComponentSerialize&, uint64_t)
    {
    // No need to serialize. But we do want that we were in the component
    // system to be serialized out.
    return true;
    }
};

} // namespace ren

#endif 
