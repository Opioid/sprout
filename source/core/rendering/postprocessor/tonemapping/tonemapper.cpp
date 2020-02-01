#include "tonemapper.hpp"

namespace rendering::postprocessor::tonemapping {

Tonemapper::~Tonemapper() = default;

void Tonemapper::init(Camera const& /*camera*/, thread::Pool& /*threads*/) noexcept {}

float Tonemapper::normalization_factor(float linear_max, float tonemapped_max) noexcept {
    return linear_max > 0.f ? 1.f / tonemapped_max : 1.f;
}

}  // namespace rendering::postprocessor::tonemapping
