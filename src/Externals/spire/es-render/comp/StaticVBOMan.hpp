#ifndef SPIRE_RENDER_COMPONENT_STATIC_VBO_MAN_HPP
#define SPIRE_RENDER_COMPONENT_STATIC_VBO_MAN_HPP

#include <es-log/trace-log.h>
#include <memory>
#include <es-cereal/ComponentSerialize.hpp>
#include <es-render/VBOMan.hpp>
#include <spire/scishare.h>

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

    bool serialize(spire::ComponentSerialize& /* s */, uint64_t /* entityID */)
    {
        // No need to serialize.
        return true;
    }
};

} // namespace ren

#endif
