#ifndef SPIRE_RENDER_COMPONENT_STATIC_FBO_MAN_HPP
#define SPIRE_RENDER_COMPONENT_STATIC_FBO_MAN_HPP

#include <es-log/trace-log.h>
#include <memory>
#include <es-cereal/ComponentSerialize.hpp>
#include "../FBOMan.hpp"

namespace ren {

struct StaticFBOMan
{
    // -- Data --
    std::shared_ptr<FBOMan> instance_;

    // -- Functions --
    StaticFBOMan() : instance_(new FBOMan) {}
    explicit StaticFBOMan(FBOMan* s) : instance_(s) {}
    static const char* getName() {return "ren:StaticFBOMan";}

    bool serialize(spire::ComponentSerialize& /* s */, uint64_t /* entityID */)
    {
        // No need to serialize.
        return true;
    }
};

} // namespace ren

#endif
