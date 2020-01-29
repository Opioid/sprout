#ifndef SU_CORE_IMAGE_PROCEDURAL_FLAKES_PROVIDER_HPP
#define SU_CORE_IMAGE_PROCEDURAL_FLAKES_PROVIDER_HPP

#include "base/math/vector2.hpp"
#include "base/math/vector3.hpp"

namespace rnd {
class Generator;
}

namespace memory {
class Variant_map;
}

namespace image {

class Image;

namespace procedural::flakes {

class Provider {
  public:
    using Variants = memory::Variant_map;

    Image* create_normal_map(Variants const& options) noexcept;

    Image* create_mask(Variants const& options) noexcept;

  private:
    struct Properties {
        Properties(Variants const& options) noexcept;

        int2 dimensions;

        uint32_t num_flakes;

        float radius;
    };

    struct Flake {
        float2 pos;
        float3 normal;
    };

    static Flake random_flake(uint32_t index, uint32_t seed, Properties const& props,
                              rnd::Generator& rng) noexcept;
};

}  // namespace procedural::flakes
}  // namespace image

#endif
