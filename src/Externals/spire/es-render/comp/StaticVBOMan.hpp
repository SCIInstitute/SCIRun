#ifndef IAUNS_RENDER_COMPONENT_STATIC_VBO_MAN_HPP
#define IAUNS_RENDER_COMPONENT_STATIC_VBO_MAN_HPP

#include <memory>
#include <es-cereal/ComponentSerialize.hpp>
#include "../VBOMan.hpp"

namespace ren {

class VBOMan;

struct StaticVBOMan
{
    // -- Data --
    std::shared_ptr<VBOMan> instance_;

    // -- Functions --
    StaticVBOMan() : instance_(std::shared_ptr<VBOMan>(new VBOMan)) {}
    StaticVBOMan(VBOMan* s) : instance_(std::shared_ptr<VBOMan>(s)) {}

    static const char* getName() {return "ren:StaticVBOMan";}

    bool serialize(CPM_ES_CEREAL_NS::ComponentSerialize& /* s */, uint64_t /* entityID */)
    {
        // No need to serialize.
        return true;
    }
};

} // namespace ren

#endif 
