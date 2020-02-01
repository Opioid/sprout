#include "postprocessor.hpp"
#include "base/math/vector4.inl"
#include "base/thread/thread_pool.hpp"
#include "image/typed_image.hpp"

namespace rendering::postprocessor {

Postprocessor::Postprocessor(uint32_t num_passes) noexcept : num_passes_(num_passes) {}

Postprocessor::~Postprocessor() noexcept = default;

bool Postprocessor::alpha_out(bool alpha_in) const {
    return alpha_in;
}

void Postprocessor::apply(image::Float4 const& source, image::Float4& destination,
                          thread::Pool& threads) {
    pre_apply(source, destination, threads);

    for (uint32_t p = 0; p < num_passes_; ++p) {
        threads.run_range(
            [this, p, &source, &destination](uint32_t id, int32_t begin, int32_t end) {
                apply(id, p, begin, end, source, destination);
            },
            0, destination.description().area());

        post_pass(p);
    }
}

void Postprocessor::pre_apply(image::Float4 const& /*source*/, image::Float4& /*destination*/,
                              thread::Pool& /*threads*/) {}

void Postprocessor::post_pass(uint32_t /*pass*/) {}

}  // namespace rendering::postprocessor
