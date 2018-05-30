#ifndef SPIRE_RENDER_COMPONENT_STATIC_TEXTURE_MAN_HPP
#define SPIRE_RENDER_COMPONENT_STATIC_TEXTURE_MAN_HPP

#include <es-log/trace-log.h>
#include <memory>
#include <es-cereal/ComponentSerialize.hpp>
#include "../TextureMan.hpp"
#include <spire/scishare.h>

namespace ren {

struct StaticTextureMan
{
    // -- Data --
    std::shared_ptr<TextureMan> instance_;

    // -- Functions --
    StaticTextureMan() : instance_(new TextureMan) {}
    explicit StaticTextureMan(TextureMan* s) : instance_(s) {}

    // This assignment operator is only used during modification calls inside
    // of the entity system. We don't care about those calls as they won't
    // affect this static shader man.
    StaticTextureMan& operator=(const StaticTextureMan&)
    {
    // We don't care about the incoming object. We've already created oun own
    // shader man and will continue to use that.
      return *this;
    }

    static const char* getName() {return "ren:StaticTextureMan";}

private:
    friend class spire::CerealHeap<StaticTextureMan>;

    bool serialize(spire::ComponentSerialize&, uint64_t)
    {
    // No need to serialize. But we do want that we were in the component
    // system to be serialized out.
      return true;
    }
};

} // namespace ren

#endif
