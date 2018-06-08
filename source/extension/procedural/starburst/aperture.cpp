#include "aperture.hpp"
#include "base/math/math.hpp"
#include "base/math/vector3.inl"

namespace procedural {
namespace starburst {

Aperture::Aperture(uint32_t num_blades, float roundness, float rotation) : roundness_(roundness) {
    blades_.resize(num_blades);

    float angle = rotation;

    for (uint32_t i = 0; i < num_blades; ++i) {
        float delta = (2.f * math::Pi) / static_cast<float>(num_blades);
        float start = static_cast<float>(i) * delta /*- 0.5f * delta*/ + angle;

        float3 b = float3(std::sin(start), std::cos(start), 0.f);
        float3 c = float3(std::sin(start + delta), std::cos(start + delta), 0.f);

        float3 cb = c - b;
        float3 n  = math::normalize(float3(cb[1], -cb[0], 0.f));
        float  d  = math::dot(n, b);

        blades_[i] = float3(n[0], n[1], d);
    }
}

float Aperture::evaluate(float2 p, float radius) const {
    float d = 0.f;

    for (auto& b : blades_) {
        float t = (b[0] * p[0] + b[1] * p[1]);  // / radius;// + radius * b.z;

        t /= b[2];

        d = std::max(d, t);
    }

    d = math::lerp(d, math::length(p), roundness_) / radius;

    if (d > 1.f) {
        d = 0.f;
    } else if (d < 1.f) {
        d = 1.f;
    }

    return d;
}

}  // namespace starburst
}  // namespace procedural
