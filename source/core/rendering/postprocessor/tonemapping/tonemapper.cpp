#include "tonemapper.hpp"

namespace rendering {
namespace postprocessor {
namespace tonemapping {

Tonemapper::~Tonemapper() {}

void Tonemapper::init(scene::camera::Camera const& /*camera*/, thread::Pool& /*pool*/) {}

float Tonemapper::normalization_factor(float linear_max, float tonemapped_max) {
    return linear_max > 0.f ? 1.f / tonemapped_max : 1.f;
}

}  // namespace tonemapping
}  // namespace postprocessor
}  // namespace rendering
