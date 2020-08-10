#ifndef SU_CORE_IMAGE_TEXTURE_ADAPTER_HPP
#define SU_CORE_IMAGE_TEXTURE_ADAPTER_HPP

#include "base/math/vector2.hpp"

namespace scene {
class Worker;
class Scene;
}  // namespace scene

namespace image::texture {

class Sampler_2D;
class Sampler_3D;

class Texture;

class Adapter {
  public:
    using Scene  = scene::Scene;
    using Worker = scene::Worker;

    Adapter();
    Adapter(uint32_t texture);
    Adapter(uint32_t texture, float scale);

    ~Adapter();

    bool operator==(Adapter const& other) const;

    bool is_valid() const;


  private:
    uint32_t texture_;

    float scale_;
};

}  // namespace image::texture

#endif
