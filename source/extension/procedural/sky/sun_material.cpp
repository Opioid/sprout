#include "sun_material.hpp"
#include "base/math/interpolated_function.inl"
#include "base/math/vector4.inl"
#include "base/spectrum/rgb.hpp"
#include "core/image/texture/texture_adapter.inl"
#include "core/image/texture/texture_float_3.hpp"
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

#include <iostream>

namespace procedural::sky {

using namespace scene;

Sun_material::Sun_material(Sky& sky) noexcept : Material(sky) {}

material::Sample const& Sun_material::sample(float3 const&      wo, Ray const& /*ray*/,
                                             Renderstate const& rs, Filter /*filter*/,
                                             Sampler& /*sampler*/, Worker const& worker) const
    noexcept {
    auto& sample = worker.sample<material::light::Sample>(rs.sample_level);

    sample.set_basis(rs.geo_n, wo);

    sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);

    sample.set(sky_.model().evaluate_sky_and_sun(-wo));

    return sample;
}

float3 Sun_material::evaluate_radiance(float3 const& wi, float2 /*uv*/, float /*area*/,
                                       Filter /*filter*/, const Worker& /*worker*/) const noexcept {
    return sky_.model().evaluate_sky_and_sun(wi);
}

float3 Sun_material::average_radiance(float /*area*/) const noexcept {
    return sky_.model().evaluate_sky_and_sun(-sky_.model().sun_direction());
}

void Sun_material::prepare_sampling(Shape const& /*shape*/, uint32_t /*part*/, uint64_t /*time*/,
                                    Transformation const& /*transformation*/, float /*area*/,
                                    bool /*importance_sampling*/, thread::Pool& /*pool*/) noexcept {
}

size_t Sun_material::num_bytes() const noexcept {
    return sizeof(*this);
}

Sun_baked_material::Sun_baked_material(Sky& sky) noexcept : Material(sky) {}

material::Sample const& Sun_baked_material::sample(float3 const&      wo, Ray const& /*ray*/,
                                                   Renderstate const& rs, Filter /*filter*/,
                                                   Sampler& /*sampler*/, Worker const& worker) const
    noexcept {
    auto& sample = worker.sample<material::light::Sample>(rs.sample_level);

    sample.set_basis(rs.geo_n, wo);

    sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);

    float3 const radiance = emission_(sky_.sun_v(-wo));

    SOFT_ASSERT(all_finite_and_positive(radiance));

    sample.set(radiance);

    return sample;
}

float3 Sun_baked_material::evaluate_radiance(float3 const& wi, float2 /*uv*/, float /*area*/,
                                             Filter /*filter*/, const Worker& /*worker*/) const
    noexcept {
    float3 const radiance = emission_(sky_.sun_v(wi));

    SOFT_ASSERT(all_finite_and_positive(radiance));

    return radiance;
}

float3 Sun_baked_material::average_radiance(float /*area*/) const noexcept {
    return sky_.model().evaluate_sky_and_sun(-sky_.model().sun_direction());
}

void Sun_baked_material::prepare_sampling(Shape const& /*shape*/, uint32_t /*part*/,
                                          uint64_t /*time*/,
                                          Transformation const& /*transformation*/, float /*area*/,
                                          bool /*importance_sampling*/,
                                          thread::Pool& /*pool*/) noexcept {
    using namespace image;

    if (!sky_.sun_changed_since_last_check()) {
        return;
    }

    static uint32_t constexpr num_samples = 1024;

    float3* cache = memory::allocate_aligned<float3>(num_samples);

    for (uint32_t i = 0; i < num_samples; ++i) {
        float const v = static_cast<float>(i) / static_cast<float>(num_samples - 1);

        float3 const radiance = sky_.model().evaluate_sky_and_sun(sky_.sun_wi(v));

        SOFT_ASSERT(all_finite_and_positive(radiance));

        cache[i] = radiance;
    }

    {
        float3 const radiance = sky_.model().evaluate_sky_and_sun(sky_.sun_wi(0.5f));

        float const lumen = spectrum::watt_to_lumen(radiance);
        std::cout << lumen << std::endl;
    }

    emission_.from_array(0.f, 1.f, num_samples, cache);

    memory::free_aligned(cache);
}

size_t Sun_baked_material::num_bytes() const noexcept {
    return sizeof(*this);
}

}  // namespace procedural::sky
