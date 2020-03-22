#include "sky_material.hpp"
#include "base/math/distribution/distribution_1d.inl"
#include "base/math/mapping.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/vector3.inl"
#include "base/memory/array.inl"
#include "base/spectrum/rgb.hpp"
#include "base/thread/thread_pool.hpp"
#include "core/image/texture/texture.inl"
#include "core/image/texture/texture_sampler.hpp"
#include "core/scene/entity/composed_transformation.hpp"
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

namespace procedural::sky {

using namespace scene;

Sky_material::Sky_material(Sky& sky) : Material(sky) {}

material::Sample const& Sky_material::sample(float3 const&      wo, scene::Ray const& /*ray*/,
                                             Renderstate const& rs, Filter /*filter*/,
                                             Sampler& /*sampler*/, Worker& worker) const {
    auto& sample = worker.sample<material::light::Sample>();

    sample.set_basis(rs.geo_n, rs.n, wo);

    sample.set(sky_.model().evaluate_sky(-wo));

    return sample;
}

float3 Sky_material::evaluate_radiance(float3 const& wi, float2 /*uv*/, float /*area*/,
                                       Filter /*filter*/, Worker const& /*worker*/) const {
    return sky_.model().evaluate_sky(wi);
}

float3 Sky_material::average_radiance(float /*area*/, Scene const& /*scene*/) const {
    return sky_.model().evaluate_sky(Model::zenith());
}

void Sky_material::prepare_sampling(Shape const& /*shape*/, uint32_t /*part*/, uint64_t /*time*/,
                                    Transformation const& /*transformation*/, float /*area*/,
                                    bool /*importance_sampling*/, thread::Pool& /*threads*/,
                                    Scene const& /*scene*/) {}

size_t Sky_material::num_bytes() const {
    return sizeof(*this);
}

static int2 constexpr Bake_dimensions(256);

Sky_baked_material::Sky_baked_material(Sky& sky)
    : Material(sky),
      cache_(image::Description(Bake_dimensions)),
      cache_texture_(image::texture::Float3(cache_)) {
    properties_.set(Property::Emission_map);
}

Sky_baked_material::~Sky_baked_material() = default;

material::Sample const& Sky_baked_material::sample(float3 const&      wo, scene::Ray const& /*ray*/,
                                                   Renderstate const& rs, Filter filter,
                                                   Sampler& /*sampler*/, Worker& worker) const {
    auto& sample = worker.sample<material::light::Sample>();

    auto const& sampler = worker.sampler_2D(sampler_key(), filter);

    sample.set_basis(rs.geo_n, rs.n, wo);

    float3 const radiance = sampler.sample_3(cache_texture_, rs.uv);

    sample.set(radiance);

    return sample;
}

float3 Sky_baked_material::evaluate_radiance(float3 const& /*wi*/, float2 uv, float /*area*/,
                                             Filter filter, Worker const& worker) const {
    auto const& sampler = worker.sampler_2D(sampler_key(), filter);

    return sampler.sample_3(cache_texture_, uv);

    //    return emission_map_.sample_3(worker, sampler, uv);
}

float3 Sky_baked_material::average_radiance(float /*area*/, Scene const& /*scene*/) const {
    return average_emission_;
}

Material::Sample_2D Sky_baked_material::radiance_sample(float2 r2) const {
    auto const result = distribution_.sample_continuous(r2);

    return {result.uv, result.pdf * total_weight_};
}

float Sky_baked_material::emission_pdf(float2 uv, Filter filter, Worker const& worker) const {
    auto& sampler = worker.sampler_2D(sampler_key(), filter);

    return distribution_.pdf(sampler.address(uv)) * total_weight_;
}

void Sky_baked_material::prepare_sampling(Shape const& shape, uint32_t /*part*/, uint64_t /*time*/,
                                          Transformation const& transformation, float /*area*/,
                                          bool importance_sampling, thread::Pool& threads,
                                          Scene const& /*scene*/) {
    using namespace image;

    if (!sky_.sky_changed_since_last_check()) {
        return;
    }

    //	std::ofstream stream("sky.png", std::ios::binary);
    //	if (stream) {
    //		image::encoding::png::Writer writer(d);
    //		writer.write(stream, *cache, threads);
    //	}

    if (importance_sampling) {
        Distribution_2D::Distribution_impl* conditional = distribution_.allocate(
            Bake_dimensions[1]);

        memory::Array<float4> artws(threads.num_threads(), float4(0.f));

        threads.run_range(
            [this, &transformation, &conditional, &artws, &shape](uint32_t id, int32_t begin,
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
                        float3 const wi = unclipped_canopy_mapping(transformation, uv);
                        float3 const li = sky_.model().evaluate_sky(wi);

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
        average_emission_ = sky_.model().evaluate_sky(Model::zenith());
    }
}

size_t Sky_baked_material::num_bytes() const {
    return sizeof(*this) + cache_.num_bytes() + distribution_.num_bytes();
}

float3 Sky_baked_material::unclipped_canopy_mapping(Transformation const& transformation,
                                                    float2                uv) {
    float2 const disk(2.f * uv[0] - 1.f, 2.f * uv[1] - 1.f);

    float3 const dir = disk_to_hemisphere_equidistant(disk);

    return transform_vector(transformation.rotation, dir);
}

}  // namespace procedural::sky
