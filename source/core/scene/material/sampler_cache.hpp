#ifndef SU_CORE_SCENE_MATERIAL_SAMPLER_CACHE_HPP
#define SU_CORE_SCENE_MATERIAL_SAMPLER_CACHE_HPP

#include <cstdint>
#include "sampler_settings.hpp"

namespace image::texture::sampler {
class Sampler_2D;
class Sampler_3D;
}  // namespace image::texture::sampler

namespace scene::material {

class Sampler_cache {
  public:
    using Filter             = Sampler_settings::Filter;
    using Texture_sampler_2D = image::texture::sampler::Sampler_2D;
    using Texture_sampler_3D = image::texture::sampler::Sampler_3D;

    Sampler_cache() noexcept;

    ~Sampler_cache() noexcept;

    Texture_sampler_2D const& sampler_2D(uint32_t key, Filter filter) const noexcept;

    Texture_sampler_3D const& sampler_3D(uint32_t key, Filter filter) const noexcept;

  private:
    static uint32_t constexpr Num_samplers = 8;

    Texture_sampler_2D* samplers_2D_[Num_samplers];
    Texture_sampler_3D* samplers_3D_[Num_samplers];
};

}  // namespace scene::material

#endif
