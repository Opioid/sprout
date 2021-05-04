#include "generic.hpp"
#include "base/math/vector4.inl"
#include "image/typed_image.hpp"

namespace rendering::postprocessor::tonemapping {

Generic::Generic(bool auto_expose, float exposure, float contrast, float shoulder, float mid_in,
                 float mid_out, float hdr_max)
    : Tonemapper(auto_expose, exposure), a_(contrast), d_(shoulder), hdr_max_(hdr_max) {
    float const ad = contrast * shoulder;

    float const midi_pow_a  = std::pow(mid_in, contrast);
    float const midi_pow_ad = std::pow(mid_in, ad);
    float const hdrm_pow_a  = std::pow(hdr_max, contrast);
    float const hdrm_pow_ad = std::pow(hdr_max, ad);

    float const u = hdrm_pow_ad * mid_out - midi_pow_ad * mid_out;
    float const v = midi_pow_ad * mid_out;

    b_ = -((-midi_pow_a + (mid_out * (hdrm_pow_ad * midi_pow_a - hdrm_pow_a * v)) / u) / v);
    c_ = (hdrm_pow_ad * midi_pow_a - hdrm_pow_a * v) / u;
}

void Generic::apply(uint32_t /*id*/, uint32_t /*pass*/, int32_t begin, int32_t end,
                    image::Float4 const& source, image::Float4& destination,
                    Scene const& /*scene*/) {
    float const factor = exposure_factor_;

    for (int32_t i = begin; i < end; ++i) {
        float4_p color = source.at(i);

        destination.store(i, float4(tonemap(factor * color[0]), tonemap(factor * color[1]),
                                    tonemap(factor * color[2]), color[3]));
    }
}

float Generic::tonemap(float x) const {
    x = std::min(x, hdr_max_);

    float const z = std::pow(x, a_);

    return z / (std::pow(z, d_) * b_ + c_);
}

}  // namespace rendering::postprocessor::tonemapping
