#ifndef IAUNS_RENDER_COMPONENT_STATIC_FBO_MAN_HPP
#define IAUNS_RENDER_COMPONENT_STATIC_FBO_MAN_HPP

#include <memory>
#include <es-cereal/ComponentSerialize.hpp>
#include "../FBOMan.hpp"

namespace ren {

struct StaticFBOMan
{
    // -- Data --
    std::shared_ptr<FBOMan> instance_;

    // -- Functions --
    StaticFBOMan() : instance_(std::shared_ptr<FBOMan>(new FBOMan)) {}
    StaticFBOMan(FBOMan* s) : instance_(std::shared_ptr<FBOMan>(s)) {}
    static const char* getName() {return "ren:StaticFBOMan";}

    bool serialize(CPM_ES_CEREAL_NS::ComponentSerialize& /* s */, uint64_t /* entityID */)
    {
        // No need to serialize.
        return true;
    }
};

} // namespace ren

#endif 
