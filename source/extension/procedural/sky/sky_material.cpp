#include "sky_material.hpp"
#include "base/math/mapping.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/vector3.inl"
#include "base/spectrum/rgb.hpp"
#include "core/image/texture/texture_adapter.inl"
#include "core/image/texture/texture_float_3.hpp"
#include "core/image/typed_image.inl"
#include "core/scene/material/light/light_material_sample.hpp"
#include "core/scene/material/material_sample.inl"
#include "core/scene/prop/prop.hpp"
#include "core/scene/scene_renderstate.hpp"
#include "core/scene/scene_worker.inl"
#include "core/scene/shape/shape.hpp"
#include "core/scene/shape/shape_sample.hpp"
#include "sky.hpp"
#include "sky_model.hpp"

//#include "core/image/encoding/png/png_writer.hpp"
//#include <fstream>
//#include <iostream>

namespace procedural::sky {

using namespace scene;

Sky_material::Sky_material(Sky& sky) noexcept : Material(sky) {}

material::Sample const& Sky_material::sample(float3 const&      wo, Ray const& /*ray*/,
                                             Renderstate const& rs, Filter /*filter*/,
                                             sampler::Sampler& /*sampler*/,
                                             Worker const& worker) const noexcept {
    auto& sample = worker.sample<material::light::Sample>(rs.sample_level);

    sample.set_basis(rs.geo_n, wo);

    sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);

    sample.set(sky_.model().evaluate_sky(-wo));

    return sample;
}

float3 Sky_material::evaluate_radiance(float3 const& wi, float2 /*uv*/, float /*area*/,
                                       Filter /*filter*/, Worker const& /*worker*/) const noexcept {
    return sky_.model().evaluate_sky(wi);
}

float3 Sky_material::average_radiance(float /*area*/) const noexcept {
    return sky_.model().evaluate_sky(sky_.model().zenith());
}

void Sky_material::prepare_sampling(Shape const& /*shape*/, uint32_t /*part*/, uint64_t /*time*/,
                                    Transformation const& /*transformation*/, float /*area*/,
                                    bool /*importance_sampling*/, thread::Pool& /*pool*/) noexcept {
}

size_t Sky_material::num_bytes() const noexcept {
    return sizeof(*this);
}

Sky_baked_material::Sky_baked_material(Sky& sky) noexcept : Material(sky) {}

Sky_baked_material::~Sky_baked_material() noexcept {}

material::Sample const& Sky_baked_material::sample(float3 const&      wo, Ray const& /*ray*/,
                                                   Renderstate const& rs, Filter filter,
                                                   sampler::Sampler& /*sampler*/,
                                                   Worker const& worker) const noexcept {
    auto& sample = worker.sample<material::light::Sample>(rs.sample_level);

    auto const& sampler = worker.sampler_2D(sampler_key(), filter);

    sample.set_basis(rs.geo_n, wo);

    sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);

    float3 const radiance = emission_map_.sample_3(sampler, rs.uv);

    sample.set(radiance);

    return sample;
}

float3 Sky_baked_material::evaluate_radiance(float3 const& /*wi*/, float2 uv, float /*area*/,
                                             Filter filter, Worker const& worker) const noexcept {
    auto const& sampler = worker.sampler_2D(sampler_key(), filter);
    return emission_map_.sample_3(sampler, uv);
}

float3 Sky_baked_material::average_radiance(float /*area*/) const noexcept {
    return average_emission_;
}

bool Sky_baked_material::has_emission_map() const noexcept {
    return true;
}

Material::Sample_2D Sky_baked_material::radiance_sample(float2 r2) const noexcept {
    auto const result = distribution_.sample_continuous(r2);

    return {result.uv, result.pdf * total_weight_};
}

float Sky_baked_material::emission_pdf(float2 uv, Filter filter, Worker const& worker) const
    noexcept {
    auto& sampler = worker.sampler_2D(sampler_key(), filter);

    return distribution_.pdf(sampler.address(uv)) * total_weight_;
}

void Sky_baked_material::prepare_sampling(Shape const& shape, uint32_t /*part*/, uint64_t /*time*/,
                                          Transformation const& transformation, float /*area*/,
                                          bool                  importance_sampling,
                                          thread::Pool& /*pool*/) noexcept {
    using namespace image;

    if (!sky_.sky_changed_since_last_check()) {
        return;
    }

    int2 const d(256);

    Image::Description const description(Image::Type::Float3, d);

    auto cache = std::make_shared<Float3>(description);

    float2 const idf = 1.f / float2(d);

    for (int32_t y = 0; y < d[1]; ++y) {
        for (int32_t x = 0; x < d[0]; ++x) {
            float2 const uv = (float2(x, y) + 0.5f) * idf;
            float3 const wi = unclipped_canopy_mapping(transformation, uv);
            float3 const li = sky_.model().evaluate_sky(wi);

            cache->store(x, y, packed_float3(li));
        }
    }

    auto cache_texture = std::make_shared<texture::Float3>(cache);

    emission_map_ = Texture_adapter(cache_texture);

    //	std::ofstream stream("sky.png", std::ios::binary);
    //	if (stream) {
    //		image::encoding::png::Writer writer(d);
    //		writer.write(stream, *cache, pool);
    //	}

    if (importance_sampling) {
        float3 average_radiance = float3::identity();

        float total_weight = 0.f;

        std::vector<float> luminance(d[0] * d[1]);

        for (int32_t y = 0, l = 0; y < d[1]; ++y) {
            for (int32_t x = 0; x < d[0]; ++x, ++l) {
                float3 const radiance = float3(cache->at(x, y));

                float2 const uv((static_cast<float>(x) + 0.5f) / static_cast<float>(d[0]),
                                (static_cast<float>(y) + 0.5f) / static_cast<float>(d[1]));

                float const weight = shape.uv_weight(uv);

                average_radiance += weight * radiance;

                total_weight += weight;

                luminance[static_cast<uint32_t>(l)] = weight * spectrum::luminance(radiance);
            }
        }

        average_emission_ = average_radiance / total_weight;

        total_weight_ = total_weight;

        distribution_.init(luminance.data(), d);
    } else {
        // This controls how often the sky will be sampled,
        // Zenith sample cause less variance in one test (favoring the sun)...
        // average_emission_ = cache_texture->average_3();
        average_emission_ = sky_.model().evaluate_sky(sky_.model().zenith());
    }
}

size_t Sky_baked_material::num_bytes() const noexcept {
    return sizeof(*this) + emission_map_.texture().image().num_bytes() + distribution_.num_bytes();
}

float3 Sky_baked_material::unclipped_canopy_mapping(Transformation const& transformation,
                                                    float2                uv) noexcept {
    float2 const disk(2.f * uv[0] - 1.f, 2.f * uv[1] - 1.f);

    float3 const dir = math::disk_to_hemisphere_equidistant(disk);

    return transform_vector(transformation.rotation, dir);
}

}  // namespace procedural::sky
