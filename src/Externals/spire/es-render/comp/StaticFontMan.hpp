#ifndef SPIRE_RENDER_COMPONENT_STATIC_FONT_MAN_HPP
#define SPIRE_RENDER_COMPONENT_STATIC_FONT_MAN_HPP

#include <es-log/trace-log.h>
#include <memory>
#include <es-cereal/ComponentSerialize.hpp>
#include "../FontMan.hpp"
#include <spire/scishare.h>

namespace ren {

struct StaticFontMan
{
    // -- Data --
    std::shared_ptr<FontMan> instance_;

    // -- Functions --
    StaticFontMan() : instance_(new FontMan) {}
    explicit StaticFontMan(FontMan* s) : instance_(s) {}

    // This assignment operator is only used during modification calls inside
    // of the entity system. We don't care about those calls as they won't
    // affect this static shader man.
    StaticFontMan& operator=(const StaticFontMan&)
    {
    // We don't care about the incoming object. We've already created oun own
    // shader man and will continue to use that.
      return *this;
    }

    static const char* getName() {return "ren:StaticFontMan";}

private:
    friend class spire::CerealHeap<StaticFontMan>;

    bool serialize(spire::ComponentSerialize&, uint64_t)
    {
    // No need to serialize. But we do want that we were in the component
    // system to be serialized out.
      return true;
    }
};

} // namespace ren

#endif
