#ifndef SU_CORE_SAMPLER_SAMPLER_HPP
#define SU_CORE_SAMPLER_SAMPLER_HPP

#include "base/math/vector.hpp"

namespace rnd {
class Generator;
}

namespace sampler {

struct Camera_sample;

class Sampler {
  public:
    Sampler(rnd::Generator& rng) noexcept;

    virtual ~Sampler() noexcept;

    void resize(uint32_t num_iterations, uint32_t num_samples_per_iteration,
                uint32_t num_dimensions_2D, uint32_t num_dimensions_1D) noexcept;

    void start_pixel() noexcept;

    rnd::Generator& rng() noexcept;

    uint32_t num_samples() const noexcept;

    virtual Camera_sample generate_camera_sample(int2 pixel, uint32_t index) noexcept = 0;

    virtual float2 generate_sample_2D(uint32_t dimension = 0) noexcept = 0;

    virtual float generate_sample_1D(uint32_t dimension = 0) noexcept = 0;

  protected:
    virtual void on_resize() noexcept = 0;

    virtual void on_start_pixel() noexcept = 0;

    rnd::Generator& rng_;

    uint32_t num_samples_;
    uint32_t num_samples_per_iteration_;

    uint32_t num_dimensions_2D_;
    uint32_t num_dimensions_1D_;

    uint32_t* current_sample_2D_;
    uint32_t* current_sample_1D_;
};

class Factory {
  public:
    Factory(uint32_t num_samplers) noexcept;

    virtual ~Factory() noexcept;

    virtual Sampler* create(uint32_t id, rnd::Generator& rng) const noexcept = 0;

  protected:
    uint32_t const num_samplers_;
};

}  // namespace sampler

#endif
