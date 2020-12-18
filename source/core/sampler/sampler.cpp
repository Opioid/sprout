#include "sampler.hpp"
#include "base/math/vector3.inl"
#include "camera_sample.hpp"

namespace sampler {

Sampler::Sampler()
    : num_samples_(0), num_dimensions_2D_(0), num_dimensions_1D_(0), current_sample_(nullptr) {}

Sampler::~Sampler() {
    delete[] current_sample_;
}

void Sampler::resize(uint32_t num_iterations, uint32_t num_samples_per_iteration,
                     uint32_t num_dimensions_2D, uint32_t num_dimensions_1D) {
    uint32_t const num_samples = num_iterations * num_samples_per_iteration;

    if (num_samples != num_samples_ || num_dimensions_2D != num_dimensions_2D_ ||
        num_dimensions_1D != num_dimensions_1D_) {
        delete[] current_sample_;

        num_samples_ = num_samples;

        num_dimensions_2D_ = num_dimensions_2D;
        num_dimensions_1D_ = num_dimensions_1D;

        current_sample_ = new uint32_t[num_dimensions_2D + num_dimensions_1D];

        on_resize();
    }
}

void Sampler::start_pixel(RNG& rng) {
    for (uint32_t i = 0, len = num_dimensions_2D_ + num_dimensions_1D_; i < len; ++i) {
        current_sample_[i] = 0;
    }

    on_start_pixel(rng);
}

Camera_sample Sampler::camera_sample(RNG& rng, int2 pixel) {
    float2 const image_sample = sample_2D(rng, 0);
    float2 const lens_sample  = sample_2D(rng, 1);
    float const  time_sample  = sample_1D(rng, 0);

    return Camera_sample{pixel, image_sample, lens_sample, time_sample};
}

Pool::Pool(uint32_t num_samplers) : num_samplers_(num_samplers) {}

Pool::~Pool() = default;

}  // namespace sampler
