#include "sampler.hpp"
#include "base/math/vector3.inl"
#include "camera_sample.hpp"

namespace sampler {

Sampler::Sampler() = default;

Sampler::~Sampler() {}

void Sampler::resize(uint32_t /*num_samples*/) {}

void Sampler::start_pixel(RNG& /*rng*/, uint32_t /*num_samples*/) {}

Camera_sample Sampler::camera_sample(RNG& rng, int2 pixel) {
    float2 const image_sample = sample_2D(rng, 0);
    float2 const lens_sample  = sample_2D(rng, 1);
    float const  time_sample  = sample_1D(rng, 0);

    return Camera_sample{pixel, image_sample, lens_sample, time_sample};
}

Buffered::Buffered(uint32_t num_dimensions_2D, uint32_t num_dimensions_1D)
    : num_dimensions_2D_(num_dimensions_2D),
      num_dimensions_1D_(num_dimensions_1D),
      num_samples_(0),
      current_sample_(new uint32_t[num_dimensions_2D + num_dimensions_1D]) {}

Buffered::~Buffered() {
    delete[] current_sample_;
}

void Buffered::resize(uint32_t num_samples) {
    if (num_samples != num_samples_) {
        num_samples_ = num_samples;

        on_resize();
    }
}

void Buffered::start_pixel(RNG& rng, uint32_t num_samples) {
    num_samples_ = num_samples;

    for (uint32_t i = 0, len = num_dimensions_2D_ + num_dimensions_1D_; i < len; ++i) {
        current_sample_[i] = 0;
    }

    on_start_pixel(rng);
}

Pool::Pool(uint32_t num_samplers) : num_samplers_(num_samplers) {}

Pool::~Pool() = default;

}  // namespace sampler
