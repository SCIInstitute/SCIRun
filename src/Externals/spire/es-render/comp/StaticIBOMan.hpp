#ifndef SPIRE_RENDER_COMPONENT_STATIC_IBO_MAN_HPP
#define SPIRE_RENDER_COMPONENT_STATIC_IBO_MAN_HPP

#include <es-log/trace-log.h>
#include <memory>
#include <es-cereal/ComponentSerialize.hpp>
#include "../IBOMan.hpp"

namespace ren {

struct StaticIBOMan
{
    // -- Data --
    std::shared_ptr<IBOMan> instance_;

    // -- Functions --
    StaticIBOMan() : instance_(new IBOMan) {}
    explicit StaticIBOMan(IBOMan* s) : instance_(s) {}
    static const char* getName() {return "ren:StaticIBOMan";}

    bool serialize(spire::ComponentSerialize& /* s */, uint64_t /* entityID */)
    {
        // No need to serialize.
        return true;
    }
};

} // namespace ren

#endif
