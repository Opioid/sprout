#include "sky_material.hpp"
#include "base/math/distribution_1d.inl"
#include "base/math/mapping.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/vector3.inl"
#include "base/memory/array.inl"
#include "base/memory/buffer.hpp"
#include "base/spectrum/rgb.hpp"
#include "base/thread/thread_pool.hpp"
#include "core/image/texture/texture.inl"
#include "core/image/texture/texture_sampler.hpp"
#include "core/resource/resource_manager.inl"
#include "core/scene/composed_transformation.hpp"
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

using namespace image;
using namespace texture;
using namespace scene;

Sky_material::Sky_material(Sky* sky) : Material(sky) {
    emission_ = float3(1.f);
}

material::Sample const& Sky_material::sample(float3_p wo, Renderstate const& rs,
                                             Sampler& /*sampler*/, Worker&   worker) const {
    auto& sample = worker.sample<material::light::Sample>();

    sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);

    float3 const radiance = sky_->model().evaluate_sky(-wo);

    sample.set_common(rs, wo, radiance, radiance, float2(0.f));

    return sample;
}

float3 Sky_material::evaluate_radiance(float3_p wi, float3_p /*n*/, float3_p /*uvw*/,
                                       float /*extent*/, Filter /*filter*/,
                                       Worker const& /*worker*/) const {
    return sky_->model().evaluate_sky(wi);
}

float3 Sky_material::prepare_sampling(Shape const& /*shape*/, uint32_t /*part*/,
                                      Transformation const& /*trafo*/, float /*area*/,
                                      Scene const& /*scene*/, Threads& /*threads*/) {
    return sky_->model().evaluate_sky(Model::zenith());
}

static int2 constexpr Bake_dimensions(256);

Sky_baked_material::Sky_baked_material(Sky* sky, Resources& resources)
    : Material(sky),
      cache_(new Image(Float3(Description(Bake_dimensions))))

{
    texture_ = Texture(Texture::Type::Float3, resources.store<Image>(cache_), float2(1.f));

    properties_.set(Property::Emission_map);
}

Sky_baked_material::~Sky_baked_material() = default;

material::Sample const& Sky_baked_material::sample(float3_p wo, Renderstate const& rs,
                                                   Sampler& /*sampler*/, Worker&   worker) const {
    auto& sample = worker.sample<material::light::Sample>();

    auto const& sampler = worker.sampler_2D(sampler_key(), rs.filter);

    sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);

    float3 const radiance = sampler.sample_3(texture_, rs.uv, worker.scene());

    sample.set_common(rs, wo, radiance, radiance, float2(0.f));

    return sample;
}

float3 Sky_baked_material::evaluate_radiance(float3_p /*wi*/, float3_p /*n*/, float3_p uvw,
                                             float /*extent*/, Filter filter,
                                             Worker const& worker) const {
    auto const& sampler = worker.sampler_2D(sampler_key(), filter);

    return sampler.sample_3(texture_, uvw.xy(), worker.scene());
}

Material::Radiance_sample Sky_baked_material::radiance_sample(float3_p r3) const {
    auto const result = distribution_.sample_continuous(r3.xy());

    return {result.uv, result.pdf * total_weight_};
}

float Sky_baked_material::emission_pdf(float3_p uvw, Worker const& worker) const {
    auto& sampler = worker.sampler_2D(sampler_key(), Filter::Undefined);

    return distribution_.pdf(sampler.address(uvw.xy())) * total_weight_;
}

float3 Sky_baked_material::prepare_sampling(Shape const&          shape, uint32_t /*part*/,
                                            Transformation const& trafo, float /*area*/,
                                            Scene const& /*scene*/, Threads& threads) {
    using namespace image;

    if (!sky_->sky_changed_since_last_check()) {
        return average_emission_;
    }

    //	std::ofstream stream("sky.png", std::ios::binary);
    //	if (stream) {
    //		image::encoding::png::Writer writer(d);
    //		writer.write(stream, *cache, threads);
    //	}

    Distribution_1D* conditional = distribution_.allocate(Bake_dimensions[1]);

    memory::Array<float4> artws(threads.num_threads(), float4(0.f));

    threads.run_range(
        [this, &trafo, &conditional, &artws, &shape](uint32_t id, int32_t begin,
                                                     int32_t end) noexcept {
            image::Float3& cache = cache_->float3();

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

    return average_emission_;
}

float3 Sky_baked_material::unclipped_canopy_mapping(Transformation const& trafo, float2 uv) {
    float2 const disk(2.f * uv[0] - 1.f, 2.f * uv[1] - 1.f);

    float3 const dir = disk_to_hemisphere_equidistant(disk);

    return transform_vector(trafo.rotation, dir);
}

}  // namespace procedural::sky
