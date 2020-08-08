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
    Sampler(rnd::Generator& rng);

    virtual ~Sampler();

    void resize(uint32_t num_iterations, uint32_t num_samples_per_iteration,
                uint16_t num_dimensions_2D, uint16_t num_dimensions_1D);

    void start_pixel();

    rnd::Generator& rng();

    uint32_t num_samples() const;

    Camera_sample generate_camera_sample(int2 pixel);

    virtual float2 generate_sample_2D(uint32_t dimension = 0) = 0;

    virtual float generate_sample_1D(uint32_t dimension = 0) = 0;

  protected:
    virtual void on_resize() = 0;

    virtual void on_start_pixel() = 0;

    rnd::Generator& rng_;

    uint32_t num_samples_;

    uint16_t num_dimensions_2D_;
    uint16_t num_dimensions_1D_;

    uint32_t* current_sample_;
};

class Pool {
  public:
    Pool(uint32_t num_samplers);

    virtual ~Pool();

    virtual Sampler* get(uint32_t id, rnd::Generator& rng) const = 0;

  protected:
    uint32_t const num_samplers_;
};

template <typename T>
class Typed_pool final : public Pool {
  public:
    Typed_pool(uint32_t num_samplers);

    ~Typed_pool() final;

    Sampler* get(uint32_t id, rnd::Generator& rng) const final;

  protected:
    T* samplers_;
};

}  // namespace sampler

#endif
