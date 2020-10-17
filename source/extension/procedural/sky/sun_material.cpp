#include "sun_material.hpp"
#include "base/math/interpolated_function_1d.inl"
#include "base/math/vector4.inl"
#include "base/spectrum/rgb.hpp"
#include "core/image/texture/texture_adapter.inl"
#include "core/image/texture/texture_float3.hpp"
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

Sun_material::Sun_material(Sky& sky) : Material(sky) {}

material::Sample const& Sun_material::sample(float3 const&      wo, Ray const& /*ray*/,
                                             Renderstate const& rs, Filter /*filter*/,
                                             Sampler& /*sampler*/, Worker& worker) const {
    auto& sample = worker.sample<material::light::Sample>();

    sample.set_basis(rs.geo_n, rs.n, wo);

    sample.set(sky_.model().evaluate_sky_and_sun(-wo));

    return sample;
}

float3 Sun_material::evaluate_radiance(float3 const& wi, float3 const& /*uvw*/, float /*extent*/,
                                       Filter /*filter*/, const Worker& /*worker*/) const {
    return sky_.model().evaluate_sky_and_sun(wi);
}

float3 Sun_material::average_radiance(float /*area*/) const {
    return sky_.model().evaluate_sky_and_sun(-sky_.model().sun_direction());
}

void Sun_material::prepare_sampling(Shape const& /*shape*/, uint32_t /*part*/, uint64_t /*time*/,
                                    Transformation const& /*transformation*/, float /*area*/,
                                    bool /*importance_sampling*/, Threads& /*threads*/,
                                    Scene const& /*scene*/) {}

Sun_baked_material::Sun_baked_material(Sky& sky) : Material(sky) {}

material::Sample const& Sun_baked_material::sample(float3 const&      wo, Ray const& /*ray*/,
                                                   Renderstate const& rs, Filter /*filter*/,
                                                   Sampler& /*sampler*/, Worker& worker) const {
    auto& sample = worker.sample<material::light::Sample>();

    sample.set_basis(rs.geo_n, rs.n, wo);

    float3 const radiance = emission_(sky_.sun_v(-wo));

    SOFT_ASSERT(all_finite_and_positive(radiance));

    sample.set(radiance);

    return sample;
}

float3 Sun_baked_material::evaluate_radiance(float3 const& wi, float3 const& /*uvw*/,
                                             float /*extent*/, Filter /*filter*/,
                                             const Worker& /*worker*/) const {
    float3 const radiance = emission_(sky_.sun_v(wi));

    SOFT_ASSERT(all_finite_and_positive(radiance));

    return radiance;
}

float3 Sun_baked_material::average_radiance(float /*area*/) const {
    return sky_.model().evaluate_sky_and_sun(-sky_.model().sun_direction());
}

void Sun_baked_material::prepare_sampling(Shape const& /*shape*/, uint32_t /*part*/,
                                          uint64_t /*time*/,
                                          Transformation const& /*transformation*/, float /*area*/,
                                          bool /*importance_sampling*/, Threads& /*threads*/,
                                          Scene const& /*scene*/) {
    using namespace image;

    if (!sky_.sun_changed_since_last_check()) {
        return;
    }

    static uint32_t constexpr num_samples = 1024;

    emission_.allocate(0.f, 1.f, num_samples);

    for (uint32_t i = 0; i < num_samples; ++i) {
        float const v = float(i) / float(num_samples - 1);

        float3 const radiance = sky_.model().evaluate_sky_and_sun(sky_.sun_wi(v));

        SOFT_ASSERT(all_finite_and_positive(radiance));

        emission_[i] = radiance;
    }
}

}  // namespace procedural::sky
