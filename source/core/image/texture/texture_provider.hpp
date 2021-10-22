#ifndef SU_CORE_IMAGE_TEXTURE_PROVIDER_HPP
#define SU_CORE_IMAGE_TEXTURE_PROVIDER_HPP

#include "base/math/vector.hpp"
#include "base/memory/variant_map.hpp"

#include <string>

namespace resource {
class Manager;
}

namespace image::texture {

class Texture;
class Texture;

class Provider {
  public:
    using Resources = resource::Manager;
    using Variants  = memory::Variant_map;

    enum class Usage { Undefined, Color, Color_with_alpha, Emission, Normal, Roughness, Surface, Mask };

    static Texture load(std::string const& filename, Variants const& options, float2 scale,
                        Resources& resources);

    static std::string encode_name(uint32_t image_id);

    static uint32_t decode_name(std::string_view name);
};

}  // namespace image::texture

#endif
