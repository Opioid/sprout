#include "sampler.hpp"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "camera_sample.hpp"

namespace sampler {

Sampler::Sampler(rnd::Generator& rng)
    : rng_(rng),
      num_samples_(0),
      num_samples_per_iteration_(0),
      num_dimensions_2D_(0),
      num_dimensions_1D_(0),
      current_sample_2D_(nullptr),
      current_sample_1D_(nullptr) {}

Sampler::~Sampler() {
    memory::free_aligned(current_sample_2D_);
}

void Sampler::resize(uint32_t num_iterations, uint32_t num_samples_per_iteration,
                     uint32_t num_dimensions_2D, uint32_t num_dimensions_1D) {
    uint32_t const num_samples = num_iterations * num_samples_per_iteration;

    if (num_samples != num_samples_ || num_samples_per_iteration != num_samples_per_iteration_ ||
        num_dimensions_2D != num_dimensions_2D_ || num_dimensions_1D != num_dimensions_1D_) {
        memory::free_aligned(current_sample_2D_);

        num_samples_               = num_samples;
        num_samples_per_iteration_ = num_samples_per_iteration;

        num_dimensions_2D_ = num_dimensions_2D;
        current_sample_2D_ = memory::allocate_aligned<uint32_t>(num_dimensions_2D +
                                                                num_dimensions_1D);

        num_dimensions_1D_ = num_dimensions_1D;
        current_sample_1D_ = current_sample_2D_ + num_dimensions_2D;

        on_resize();
    }
}

void Sampler::start_pixel() {
    for (uint32_t i = 0, len = num_dimensions_2D_ + num_dimensions_1D_; i < len; ++i) {
        current_sample_2D_[i] = 0;
    }

    on_start_pixel();
}

rnd::Generator& Sampler::rng() {
    return rng_;
}

uint32_t Sampler::num_samples() const {
    return num_samples_;
}

Camera_sample Sampler::generate_camera_sample(int2 pixel) {
    float2 const image_sample = generate_sample_2D(0);
    float2 const lens_sample  = generate_sample_2D(1);
    float const  time_sample  = generate_sample_1D(0);

    return Camera_sample{pixel, image_sample, lens_sample, time_sample};
}

Pool::Pool(uint32_t num_samplers) : num_samplers_(num_samplers) {}

Pool::~Pool() = default;

}  // namespace sampler
