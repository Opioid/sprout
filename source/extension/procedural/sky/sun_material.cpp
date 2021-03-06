#include "sun_material.hpp"
#include "base/math/interpolated_function_1d.inl"
#include "base/math/vector4.inl"
#include "base/spectrum/rgb.hpp"
#include "core/image/typed_image.hpp"
#include "core/scene/material/light/light_material_sample.hpp"
#include "core/scene/material/material_sample.inl"
#include "core/scene/prop/prop.hpp"
#include "core/scene/scene_renderstate.hpp"
#include "core/scene/scene_worker.inl"
#include "core/scene/shape/shape.hpp"
#include "core/scene/shape/shape_sample.hpp"
#include "sky.hpp"
#include "sky_model.hpp"

#include "base/debug/assert.hpp"

namespace procedural::sky {

using namespace scene;

Sun_material::Sun_material(Sky* sky) : Material(sky) {
    emission_ = float3(1.f);
}

material::Sample const& Sun_material::sample(float3_p wo, Renderstate const& rs,
                                             Sampler& /*sampler*/, Worker&   worker) const {
    auto& sample = worker.sample<material::light::Sample>();

    float3 const radiance = sky_->model().evaluate_sky_and_sun(-wo);

    sample.set_common(rs, wo, radiance, radiance, float2(0.f));

    return sample;
}

float3 Sun_material::evaluate_radiance(float3_p wi, float3_p /*n*/, float3_p /*uvw*/,
                                       float /*extent*/, Filter /*filter*/,
                                       const Worker& /*worker*/) const {
    return sky_->model().evaluate_sky_and_sun(wi);
}

float3 Sun_material::prepare_sampling(Shape const& /*shape*/, uint32_t /*part*/,
                                      Transformation const& /*trafo*/, float /*area*/,
                                      Scene const& /*scene*/, Threads& /*threads*/) {
    return sky_->model().evaluate_sky_and_sun(-sky_->model().sun_direction());
}

Sun_baked_material::Sun_baked_material(Sky* sky) : Material(sky) {
    emission_ = float3(1.f);
}

material::Sample const& Sun_baked_material::sample(float3_p wo, Renderstate const& rs,
                                                   Sampler& /*sampler*/, Worker&   worker) const {
    auto& sample = worker.sample<material::light::Sample>();

    float3 const radiance = sun_emission_(sky_->sun_v(-wo));

    SOFT_ASSERT(all_finite_and_positive(radiance));

    sample.set_common(rs, wo, radiance, radiance, float2(0.f));

    return sample;
}

float3 Sun_baked_material::evaluate_radiance(float3_p wi, float3_p /*n*/, float3_p /*uvw*/,
                                             float /*extent*/, Filter /*filter*/,
                                             const Worker& /*worker*/) const {
    float3 const radiance = sun_emission_(sky_->sun_v(wi));

    SOFT_ASSERT(all_finite_and_positive(radiance));

    return radiance;
}

float3 Sun_baked_material::prepare_sampling(Shape const& /*shape*/, uint32_t /*part*/,
                                            Transformation const& /*trafo*/, float /*area*/,
                                            Scene const& /*scene*/, Threads& /*threads*/
) {
    if (sky_->sun_changed_since_last_check()) {
        static uint32_t constexpr Num_samples = 1024;

        sun_emission_.allocate(0.f, 1.f, Num_samples);

        for (uint32_t i = 0; i < Num_samples; ++i) {
            float const v = float(i) / float(Num_samples - 1);

            float3 const radiance = sky_->model().evaluate_sky_and_sun(sky_->sun_wi(v));

            SOFT_ASSERT(all_finite_and_positive(radiance));

            sun_emission_[i] = radiance;
        }
    }

    return sky_->model().evaluate_sky_and_sun(-sky_->model().sun_direction());
}

}  // namespace procedural::sky
