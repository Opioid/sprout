#include "postprocessor.hpp"
#include "base/math/vector4.inl"
#include "base/thread/thread_pool.hpp"
#include "image/typed_image.inl"

namespace rendering::postprocessor {

Postprocessor::Postprocessor(uint32_t num_passes) : num_passes_(num_passes) {}

Postprocessor::~Postprocessor() {}

bool Postprocessor::alpha_out(bool alpha_in) const {
    return alpha_in;
}

void Postprocessor::apply(image::Float4 const& source, image::Float4& destination,
                          thread::Pool& pool) {
    pre_apply(source, destination, pool);

    for (uint32_t p = 0; p < num_passes_; ++p) {
        pool.run_range(
            [this, p, &source, &destination](uint32_t id, int32_t begin, int32_t end) {
                apply(id, p, begin, end, source, destination);
            },
            0, destination.area());
    }
}

void Postprocessor::pre_apply(image::Float4 const& /*source*/, image::Float4& /*destination*/,
                              thread::Pool& /*pool*/) {}

}  // namespace rendering::postprocessor
