#include "camera_stereoscopic.hpp"
#include "base/math/vector4.inl"

namespace scene::camera {

Stereoscopic::Stereoscopic() {
    set_interpupillary_distance(0.062f);
}

Stereoscopic::~Stereoscopic() = default;

void Stereoscopic::set_interpupillary_distance(float ipd) {
    float const half_ipd = 0.5f * ipd;

    eye_offsets_[0] = float3(-half_ipd, 0.f, 0.f);
    eye_offsets_[1] = float3(half_ipd, 0.f, 0.f);
}

}  // namespace scene::camera
