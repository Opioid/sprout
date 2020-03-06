#include "tonemapper.hpp"

#include <cmath>

namespace rendering::postprocessor::tonemapping {

Tonemapper::Tonemapper(float exposure) : exposure_factor_(std::exp2(exposure)) {}

Tonemapper::~Tonemapper() = default;

void Tonemapper::init(Camera const& /*camera*/, thread::Pool& /*threads*/) {}

float Tonemapper::normalization_factor(float linear_max, float tonemapped_max) {
    return linear_max > 0.f ? 1.f / tonemapped_max : 1.f;
}

}  // namespace rendering::postprocessor::tonemapping
