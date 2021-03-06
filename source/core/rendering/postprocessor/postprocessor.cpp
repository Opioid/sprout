#include "postprocessor.hpp"
#include "base/math/vector4.inl"
#include "base/thread/thread_pool.hpp"
#include "image/typed_image.hpp"

namespace rendering::postprocessor {

Postprocessor::Postprocessor(uint32_t num_passes) : num_passes_(num_passes) {}

Postprocessor::~Postprocessor() = default;

bool Postprocessor::alpha_out(bool alpha_in) const {
    return alpha_in;
}

void Postprocessor::apply(image::Float4 const& source, image::Float4& destination,
                          Scene const& scene, Threads& threads) {
    pre_apply(source, destination, threads);

    for (uint32_t p = 0; p < num_passes_; ++p) {
        threads.run_range(
            [this, p, &source, &destination, &scene](uint32_t id, int32_t begin, int32_t end) {
                apply(id, p, begin, end, source, destination, scene);
            },
            0, int32_t(destination.description().num_pixels()));

        post_pass(p);
    }
}

void Postprocessor::pre_apply(image::Float4 const& /*source*/, image::Float4& /*destination*/,
                              Threads& /*threads*/) {}

void Postprocessor::post_pass(uint32_t /*pass*/) {}

}  // namespace rendering::postprocessor
