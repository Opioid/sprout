#ifndef SU_CORE_SAMPLER_SAMPLER_HPP
#define SU_CORE_SAMPLER_SAMPLER_HPP

#include "base/math/vector.hpp"

namespace rnd {
class Generator;
}

using RNG = rnd::Generator;

namespace sampler {

struct Camera_sample;

class Sampler {
  public:
    Sampler();

    virtual ~Sampler();

    virtual void resize(uint32_t num_iterations, uint32_t num_samples_per_iteration,
                        uint32_t num_dimensions_2D, uint32_t num_dimensions_1D);

    virtual void start_pixel(RNG& rng);

    Camera_sample camera_sample(RNG& rng, int2 pixel);

    virtual float2 sample_2D(RNG& rng, uint32_t dimension = 0) = 0;

    virtual float sample_1D(RNG& rng, uint32_t dimension = 0) = 0;
};

class Buffered : public Sampler {
  public:
    Buffered();

    ~Buffered() override;

    void resize(uint32_t num_iterations, uint32_t num_samples_per_iteration,
                uint32_t num_dimensions_2D, uint32_t num_dimensions_1D) final;

    void start_pixel(RNG& rng) final;

  protected:
    virtual void on_resize() = 0;

    virtual void on_start_pixel(RNG& rng) = 0;

    uint32_t num_samples_;

    uint32_t num_dimensions_2D_;
    uint32_t num_dimensions_1D_;

    uint32_t* current_sample_;
};

class Pool {
  public:
    Pool(uint32_t num_samplers);

    virtual ~Pool();

    virtual Sampler* get(uint32_t id) const = 0;

  protected:
    uint32_t const num_samplers_;
};

template <typename T>
class Typed_pool final : public Pool {
  public:
    Typed_pool(uint32_t num_samplers);

    ~Typed_pool() final;

    Sampler* get(uint32_t id) const final;

  protected:
    T* samplers_;
};

}  // namespace sampler

#endif
