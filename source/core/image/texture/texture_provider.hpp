#ifndef SU_CORE_IMAGE_TEXTURE_PROVIDER_HPP
#define SU_CORE_IMAGE_TEXTURE_PROVIDER_HPP

#include "resource/resource_provider.hpp"
#include "base/math/vector.hpp"

namespace image {

class Image;

namespace texture {

class Texture;
class Turbotexture;

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
        Mask
    };

    Texture* load(std::string const& filename, Variants const& options, Resources& resources,
                  std::string& resolved_name) final;

    Texture* load(void const* data, std::string const& source_name, Variants const& options,
                  Resources& resources) final;

    static Turbotexture loadly(std::string const& filename, Variants const& options, float2 scale, Resources& resources);

    static std::string encode_name(uint32_t image_id);

    static uint32_t decode_name(std::string_view name);

  private:
    bool no_textures_;
};

}  // namespace texture
}  // namespace image

#endif
