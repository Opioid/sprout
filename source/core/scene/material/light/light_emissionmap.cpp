#include "light_emissionmap.hpp"
#include "base/math/distribution/distribution_1d.inl"
#include "base/math/math.hpp"
#include "base/math/vector4.inl"
#include "base/memory/array.inl"
#include "base/memory/buffer.hpp"
#include "base/spectrum/rgb.hpp"
#include "base/thread/thread_pool.hpp"
#include "image/texture/texture.inl"
#include "image/texture/texture_adapter.inl"
#include "light_material_sample.hpp"
#include "scene/material/material.inl"
#include "scene/material/material_sample.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"
#include "scene/shape/shape.hpp"

// https://cgg.mff.cuni.cz/~jaroslav/papers/2019-mis-compensation/2019-karlik-mis-compensation-paper.pdf
// http://www.iliyan.com/publications/Siggraph2020Course/Siggraph2020Course_Notes.pdf
// https://twitter.com/VrKomarov/status/1297454856177954816

namespace scene::material::light {

Emissionmap::Emissionmap(Sampler_settings const& sampler_settings, bool two_sided)
    : Material(sampler_settings, two_sided), average_emission_(float3(-1.f)) {}

Emissionmap::~Emissionmap() = default;

material::Sample const& Emissionmap::sample(float3 const&      wo, Ray const& /*ray*/,
                                            Renderstate const& rs, Filter filter,
                                            Sampler& /*sampler*/, Worker& worker) const {
    auto& sample = worker.sample<Sample>();

    auto& sampler = worker.sampler_2D(sampler_key(), filter);

    sample.set_basis(rs.geo_n, rs.n, wo);

    float3 const radiance = emission_map_.sample_3(worker, sampler, rs.uv);

    sample.set(emission_factor_ * radiance);

    return sample;
}

float3 Emissionmap::evaluate_radiance(float3 const& /*wi*/, float3 const& uvw, float /*extent*/,
                                      Filter filter, Worker const& worker) const {
    auto& sampler = worker.sampler_2D(sampler_key(), filter);

    return emission_factor_ * emission_map_.sample_3(worker, sampler, uvw.xy());
}

float3 Emissionmap::average_radiance(float /*area*/) const {
    return average_emission_;
}

Material::Radiance_sample Emissionmap::radiance_sample(float3 const& r3) const {
    auto const result = distribution_.sample_continuous(r3.xy());

    return {result.uv, result.pdf * total_weight_};
}

float Emissionmap::emission_pdf(float3 const& uvw, Filter filter, Worker const& worker) const {
    auto& sampler = worker.sampler_2D(sampler_key(), filter);

    return distribution_.pdf(sampler.address(uvw.xy())) * total_weight_;
}

void Emissionmap::prepare_sampling(Shape const& shape, uint32_t /*part*/, uint64_t /*time*/,
                                   Transformation const& /*transformation*/, float /*area*/,
                                   bool importance_sampling, thread::Pool& threads,
                                   Scene const& scene) {
    if (average_emission_[0] >= 0.f) {
        // Hacky way to check whether prepare_sampling has been called before
        // average_emission_ is initialized with negative values...
        return;
    }

    prepare_sampling_internal(shape, 0, importance_sampling, threads, scene);
}

void Emissionmap::set_emission_map(Texture_adapter const& emission_map) {
    emission_map_ = emission_map;

    properties_.set(Property::Emission_map, emission_map.is_valid());
}

void Emissionmap::set_emission_factor(float emission_factor) {
    emission_factor_ = emission_factor;
}

void Emissionmap::prepare_sampling_internal(Shape const& shape, int32_t element,
                                            bool importance_sampling, thread::Pool& threads,
                                            Scene const& scene) {
    auto const& texture = emission_map_.texture(scene);

    if (importance_sampling) {
        auto const d = texture.dimensions().xy();

        memory::Buffer<float> luminance(d[0] * d[1]);

        memory::Array<float4> avgs(threads.num_threads(), float4(0.f));

        threads.run_range(
            [&luminance, &avgs, &shape, &texture, element](uint32_t id, int32_t begin,
                                                           int32_t end) noexcept {
                auto const d = texture.dimensions().xy();

                float2 const idf = 1.f / float2(d);

                float4 avg(0.f);

                for (int32_t y = begin; y < end; ++y) {
                    float const v = idf[1] * (float(y) + 0.5f);

                    int32_t const row = y * d[0];

                    for (int32_t x = 0; x < d[0]; ++x) {
                        float const u = idf[0] * (float(x) + 0.5f);

                        float const uv_weight = shape.uv_weight(float2(u, v));

                        float3 const radiance = texture.at_element_3(x, y, element);

                        float3 const wr = uv_weight * radiance;

                        avg += float4(wr, uv_weight);

                        luminance[row + x] = spectrum::luminance(wr);
                    }
                }

                avgs[id] += avg;
            },
            0, d[1]);

        float4 avg(0.f);
        for (auto const& a : avgs) {
            avg += a;
        }

        float3 const average_emission = avg.xyz() / avg[3];

        average_emission_ = emission_factor_ * average_emission;

        total_weight_ = avg[3];

        float const al = 0.6f * spectrum::luminance(average_emission);

        Distribution_1D* conditional = distribution_.allocate(uint32_t(d[1]));

        threads.run_range(
            [conditional, al, &luminance, d](uint32_t /*id*/, int32_t begin, int32_t end) noexcept {
                for (int32_t y = begin; y < end; ++y) {
                    float* luminance_row = luminance.data() + (y * d[0]);

                    for (int32_t x = 0; x < d[0]; ++x) {
                        float const l = luminance_row[x];

#ifdef SU_IBL_MIS_COMPENSATION
                        float const p = std::max(l - al, 0.f);
#else
                        float const p = l;
#endif

                        luminance_row[x] = p;
                    }

                    conditional[y].init(luminance_row, uint32_t(d[0]));
                }
            },
            0, d[1]);

        distribution_.init();
    } else {
        average_emission_ = emission_factor_ * texture.average_3();
    }

    if (is_two_sided()) {
        average_emission_ *= 2.f;
    }
}

}  // namespace scene::material::light
