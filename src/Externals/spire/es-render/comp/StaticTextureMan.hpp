#ifndef IAUNS_RENDER_COMPONENT_STATIC_TEXTURE_MAN_HPP
#define IAUNS_RENDER_COMPONENT_STATIC_TEXTURE_MAN_HPP

#include <memory>
#include <es-cereal/ComponentSerialize.hpp>
#include "../TextureMan.hpp"

namespace ren {

struct StaticTextureMan
{
    // -- Data --
    std::shared_ptr<TextureMan> instance_;

    // -- Functions --
    StaticTextureMan() : instance_(std::shared_ptr<TextureMan>(new TextureMan)) {}
    StaticTextureMan(TextureMan* s) : instance_(std::shared_ptr<TextureMan>(s)) {}

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
    friend class CPM_ES_CEREAL_NS::CerealHeap<StaticTextureMan>;

    bool serialize(CPM_ES_CEREAL_NS::ComponentSerialize&, uint64_t)
    {
    // No need to serialize. But we do want that we were in the component
    // system to be serialized out.
    return true;
    }
};

} // namespace ren

#endif 
