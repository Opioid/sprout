#ifndef SU_CORE_IMAGE_PROCEDURAL_FLAKES_PROVIDER_HPP
#define SU_CORE_IMAGE_PROCEDURAL_FLAKES_PROVIDER_HPP

#include <memory>
#include "base/math/vector2.hpp"

namespace memory {
class Variant_map;
}

namespace image {

class Image;

namespace procedural::flakes {

class Provider {
  public:
    std::shared_ptr<Image> create_normal_map(memory::Variant_map const& options);

    std::shared_ptr<Image> create_mask(memory::Variant_map const& options);

  private:
    struct Properties {
        Properties(memory::Variant_map const& options);

        int2 dimensions;

        uint32_t num_flakes;

        float radius;
        float variance;
    };
};

}  // namespace procedural::flakes
}  // namespace image

#endif
