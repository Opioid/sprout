#include "substitute_subsurface_sample.hpp"
#include "base/math/math.hpp"
#include "base/math/sampling/sampling.hpp"
#include "base/math/vector4.inl"
#include "sampler/sampler.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_sample.inl"
#include "substitute_base_sample.inl"

namespace scene::material::substitute {

bxdf::Result Sample_subsurface::evaluate(f_float3 wi, bool avoid_caustics) const {
    if (!same_hemisphere(wo_)) {
        return {float3::identity(), 0.f};
    }

    float3 const h        = math::normalize(wo_ + wi);
    float const  wo_dot_h = clamp_dot(wo_, h);

    auto result = layer_.base_evaluate(wi, wo_, h, wo_dot_h, avoid_caustics);
    result.pdf *= 0.5f;
    return result;
}

void Sample_subsurface::sample(sampler::Sampler& sampler, bool avoid_caustics,
                               bxdf::Sample& result) const {
    bool const same_side = same_hemisphere(wo_);

    float const p = sampler.generate_sample_1D();

    if (same_side) {
        if (p < 0.5f) {
            refract(same_side, layer_, sampler, result);
        } else {
            if (p < 0.75f) {
                layer_.diffuse_sample(wo_, sampler, avoid_caustics, result);
            } else {
                layer_.specular_sample(wo_, sampler, result);
            }
        }
    } else {
        Layer tmp_layer = layer_;
        tmp_layer.n_    = -layer_.n_;

        if (p < 0.5f) {
            refract(same_side, tmp_layer, sampler, result);
        } else {
            reflect_internally(tmp_layer, sampler, result);
        }
    }

    result.pdf *= 0.5f;
    result.wavelength = 0.f;
}

void Sample_subsurface::set(float anisotropy, IOR const& ior) {
    anisotropy_ = anisotropy;
    ior_        = ior;
}

void Sample_subsurface::refract(bool same_side, Layer const& layer, sampler::Sampler& sampler,
                                bxdf::Sample& result) const {
    IOR tmp_ior;

    if (same_side) {
        tmp_ior.ior_i_ = ior_.ior_i_;
        tmp_ior.ior_o_ = ior_.ior_o_;
        tmp_ior.eta_i_ = ior_.eta_i_;
    } else {
        tmp_ior.ior_i_ = ior_.ior_o_;
        tmp_ior.ior_o_ = ior_.ior_i_;
        tmp_ior.eta_i_ = ior_.eta_t_;
    }

    float const n_dot_wo = layer.clamp_abs_n_dot(wo_);

    fresnel::Schlick const schlick(layer.f0_);
    float const n_dot_wi = ggx::Isotropic::refract(wo_, n_dot_wo, layer, tmp_ior, schlick, sampler,
                                                   result);

    result.reflection *= n_dot_wi;
}

void Sample_subsurface::reflect_internally(Layer const& layer, sampler::Sampler& sampler,
                                           bxdf::Sample& result) const {
    IOR tmp_ior;

    tmp_ior.ior_i_ = ior_.ior_o_;
    tmp_ior.ior_o_ = ior_.ior_i_;
    tmp_ior.eta_i_ = ior_.eta_t_;

    float const n_dot_wo = layer.clamp_abs_n_dot(wo_);

    fresnel::Schlick const schlick(layer.f0_);
    float const n_dot_wi = ggx::Isotropic::reflect_internally(wo_, n_dot_wo, layer, tmp_ior,
                                                              schlick, sampler, result);

    SOFT_ASSERT(testing::check(result, wo_, layer));

    result.reflection *= n_dot_wi;
}

}  // namespace scene::material::substitute
