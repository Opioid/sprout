#ifndef SU_CORE_IMAGE_TEXTURE_PROVIDER_HPP
#define SU_CORE_IMAGE_TEXTURE_PROVIDER_HPP

#include "resource/resource_provider.hpp"

namespace image {

class Image;

namespace texture {

class Texture;

class Provider final : public resource::Provider<Texture> {
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

    Texture* load(std::string const& filename, Variants const& options, Resources& resources,
                  std::string& resolved_name) final;

    Texture* load(void const* data, std::string const& source_name, Variants const& options,
                  Resources& resources) final;

    static std::string encode_name(uint32_t image_id);

    static uint32_t decode_name(std::string const& name);

  private:
    bool no_textures_;
};

}  // namespace texture
}  // namespace image

#endif
