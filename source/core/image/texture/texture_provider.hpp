#ifndef SU_CORE_IMAGE_TEXTURE_PROVIDER_HPP
#define SU_CORE_IMAGE_TEXTURE_PROVIDER_HPP

#include "resource/resource_provider.hpp"

namespace image {

class Image;

namespace texture {

class Texture;

class Provider final : public resource::Provider<Texture> {
  public:
    Provider() noexcept;

    enum class Usage {
        Undefined,
        Color,
        Normal,
        Anisotropy,
        Roughness,
        Surface,
        Specularity,
        Mask
    };

    Texture* load(std::string const& filename, Variant_map const& options,
                  resource::Manager& manager) noexcept override final;

    Texture* load(void const* data, std::string_view mount_folder, Variant_map const& options,
                  resource::Manager& manager) noexcept override final;

    size_t num_bytes() const noexcept override final;
};

}  // namespace texture
}  // namespace image

#endif
