#ifndef SU_EXTENSION_PROCEDURAL_FLUID_VOLUME_RENDERER_HPP
#define SU_EXTENSION_PROCEDURAL_FLUID_VOLUME_RENDERER_HPP

#include "base/math/vector3.hpp"
#include "image/typed_image_fwd.hpp"

namespace procedural::fluid {

class Volume_renderer {
  public:
    using Type = float3;

    Volume_renderer(int3 const& dimensions, uint32_t max_saturation) noexcept;

    ~Volume_renderer() noexcept;

    void resolve(image::Byte3& target) const noexcept;

    void resolve(image::Float1& target) const noexcept;

    void clear() noexcept;

    void splat(float3 const& uvw, Type const& value) noexcept;

  private:
    void splat(int3 const& c, Type const& value) noexcept;

    int3 dimensions_;

    Type* voxels_;

    float max_saturation_;
};

}  // namespace procedural::fluid

#endif
