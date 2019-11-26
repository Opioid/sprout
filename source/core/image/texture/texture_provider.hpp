#ifndef SU_CORE_IMAGE_TEXTURE_PROVIDER_HPP
#define SU_CORE_IMAGE_TEXTURE_PROVIDER_HPP

#include "resource/resource_provider.hpp"

namespace image {

class Image;

namespace texture {

class Texture;

class Provider final : public resource::Provider<Texture> {
  public:
    Provider(bool no_textures) noexcept;

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

    Texture* load(std::string const& filename, Variant_map const& options,
                  resource::Manager& resources, std::string& resolved_name) noexcept override final;

    Texture* load(void const* data, std::string const& source_name, Variant_map const& options,
                  resource::Manager& resources) noexcept override final;

    size_t num_bytes() const noexcept override final;

    size_t num_bytes(Texture const* resource) const noexcept override final;

  private:
    bool no_textures_;
};

}  // namespace texture
}  // namespace image

#endif
