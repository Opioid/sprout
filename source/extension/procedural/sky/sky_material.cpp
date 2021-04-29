#include "sky_material.hpp"
#include "base/math/distribution/distribution_1d.inl"
#include "base/math/mapping.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/vector3.inl"
#include "base/memory/array.inl"
#include "base/memory/buffer.hpp"
#include "base/spectrum/rgb.hpp"
#include "base/thread/thread_pool.hpp"
#include "core/image/texture/texture.inl"
#include "core/image/texture/texture_sampler.hpp"
#include "core/scene/entity/composed_transformation.hpp"
#include "core/scene/material/light/light_material_sample.hpp"
#include "core/scene/material/material.inl"
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

namespace procedural::sky {

using namespace scene;

Sky_material::Sky_material(Sky* sky) : Material(sky) {}

material::Sample const& Sky_material::sample(float3_p           wo, scene::Ray const& /*ray*/,
                                             Renderstate const& rs, Filter /*filter*/,
                                             Sampler& /*sampler*/, Worker& worker) const {
    auto& sample = worker.sample<material::light::Sample>();

    sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);

    float3 const radiance = sky_->model().evaluate_sky(-wo);

    sample.set_common(rs, wo, radiance, radiance, 0.f);

    return sample;
}

float3 Sky_material::evaluate_radiance(float3_p wi, float3_p /*uvw*/, float /*extent*/,
                                       Filter /*filter*/, Worker const& /*worker*/) const {
    return sky_->model().evaluate_sky(wi);
}

float3 Sky_material::average_radiance(float /*area*/) const {
    return sky_->model().evaluate_sky(Model::zenith());
}

void Sky_material::prepare_sampling(Shape const& /*shape*/, uint32_t /*part*/,
                                    Transformation const& /*trafo*/, float /*area*/,
                                    bool /*importance_sampling*/, Threads& /*threads*/,
                                    Scene const& /*scene*/) {}

static int2 constexpr Bake_dimensions(256);

Sky_baked_material::Sky_baked_material(Sky* sky)
    : Material(sky),
      cache_(image::Description(Bake_dimensions)),
      cache_texture_(image::texture::Float3(cache_), 0xFFFFFFFF) {
    properties_.set(Property::Emission_map);
}

Sky_baked_material::~Sky_baked_material() = default;

material::Sample const& Sky_baked_material::sample(float3_p           wo, scene::Ray const& /*ray*/,
                                                   Renderstate const& rs, Filter filter,
                                                   Sampler& /*sampler*/, Worker& worker) const {
    auto& sample = worker.sample<material::light::Sample>();

    auto const& sampler = worker.sampler_2D(sampler_key(), filter);

    sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);

    float3 const radiance = sampler.sample_3(cache_texture_, rs.uv);

    sample.set_common(rs, wo, radiance, radiance, 0.f);

    return sample;
}

float3 Sky_baked_material::evaluate_radiance(float3_p /*wi*/, float3_p uvw, float /*extent*/,
                                             Filter filter, Worker const& worker) const {
    auto const& sampler = worker.sampler_2D(sampler_key(), filter);

    return sampler.sample_3(cache_texture_, uvw.xy());

    //    return emission_map_.sample_3(worker, sampler, uvw.xy());
}

float3 Sky_baked_material::average_radiance(float /*area*/) const {
    return average_emission_;
}

Material::Radiance_sample Sky_baked_material::radiance_sample(float3_p r3) const {
    auto const result = distribution_.sample_continuous(r3.xy());

    return {result.uv, result.pdf * total_weight_};
}

float Sky_baked_material::emission_pdf(float3_p uvw, Filter filter, Worker const& worker) const {
    auto& sampler = worker.sampler_2D(sampler_key(), filter);

    return distribution_.pdf(sampler.address(uvw.xy())) * total_weight_;
}

void Sky_baked_material::prepare_sampling(Shape const&          shape, uint32_t /*part*/,
                                          Transformation const& trafo, float /*area*/,
                                          bool importance_sampling, Threads& threads,
                                          Scene const& /*scene*/) {
    using namespace image;

    if (!sky_->sky_changed_since_last_check()) {
        return;
    }

    //	std::ofstream stream("sky.png", std::ios::binary);
    //	if (stream) {
    //		image::encoding::png::Writer writer(d);
    //		writer.write(stream, *cache, threads);
    //	}

    if (importance_sampling) {
        Distribution_1D* conditional = distribution_.allocate(Bake_dimensions[1]);

        memory::Array<float4> artws(threads.num_threads(), float4(0.f));

        threads.run_range(
            [this, &trafo, &conditional, &artws, &shape](uint32_t id, int32_t begin,
                                                         int32_t end) noexcept {
                image::Float3& cache = cache_;

                float2 const idf = 1.f / float2(Bake_dimensions);

                auto luminance = memory::Buffer<float>(Bake_dimensions[0]);

                float4 artw(0.f);

                for (int32_t y = begin; y < end; ++y) {
                    float const v = idf[1] * (y + 0.5f);

                    for (int32_t x = 0; x < Bake_dimensions[0]; ++x) {
                        float const u = idf[0] * (x + 0.5f);

                        float2 const uv = float2(u, v);
                        float3 const wi = unclipped_canopy_mapping(trafo, uv);
                        float3 const li = sky_->model().evaluate_sky(wi);

                        cache.store(x, y, packed_float3(li));

                        float const uv_weight = shape.uv_weight(float2(u, v));

                        float3 const wli = uv_weight * li;

                        luminance[x] = spectrum::luminance(wli);

                        artw += float4(wli, uv_weight);
                    }

                    conditional[y].init(luminance.data(), Bake_dimensions[0]);
                }

                artws[id] += artw;
            },
            0, Bake_dimensions[1]);

        float4 artw(0.f);
        for (auto& a : artws) {
            artw += a;
        }

        average_emission_ = artw.xyz() / artw[3];

        total_weight_ = artw[3];

        distribution_.init();
    } else {
        // This controls how often the sky will be sampled,
        // Zenith sample cause less variance in one test (favoring the sun)...
        // average_emission_ = cache_texture->average_3();
        average_emission_ = sky_->model().evaluate_sky(Model::zenith());
    }
}

float3 Sky_baked_material::unclipped_canopy_mapping(Transformation const& trafo, float2 uv) {
    float2 const disk(2.f * uv[0] - 1.f, 2.f * uv[1] - 1.f);

    float3 const dir = disk_to_hemisphere_equidistant(disk);

    return transform_vector(trafo.rotation, dir);
}

}  // namespace procedural::sky
