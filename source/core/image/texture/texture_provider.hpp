#ifndef SU_CORE_IMAGE_TEXTURE_PROVIDER_HPP
#define SU_CORE_IMAGE_TEXTURE_PROVIDER_HPP

#include "resource/resource_manager.hpp"
#include "memory/variant_map.hpp"

namespace image {

class Image;

namespace texture {

class Texture;

class Provider {
  public:
    Provider(bool no_textures);

    enum class Usage {
        Undefined,
        Color,
        Color_with_alpha,
        Normal,
        Anisotropy,
        Roughness,
        Surface,
        Gloss,
        Gloss_in_alpha,
        Mask
    };

    using Resources = resource::Manager;
    using Variants = memory::Variant_map;

    Texture load(std::string const& filename, float scale, Variants const& options, Resources& resources) const;

    static std::string encode_name(uint32_t image_id);

    static uint32_t decode_name(std::string const& name);

  private:
    bool no_textures_;
};

}  // namespace texture
}  // namespace image

#endif
