#include "light_emissionmap.hpp"
#include "base/math/distribution/distribution_1d.inl"
#include "base/math/math.hpp"
#include "base/math/vector4.inl"
#include "base/memory/array.inl"
#include "base/spectrum/rgb.hpp"
#include "base/thread/thread_pool.hpp"
#include "image/texture/texture.inl"
#include "image/texture/texture_adapter.inl"
#include "light_material_sample.hpp"
#include "scene/material/material_sample.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"
#include "scene/shape/shape.hpp"

namespace scene::material::light {

Emissionmap::Emissionmap(Sampler_settings const& sampler_settings, bool two_sided)
    : Material(sampler_settings, two_sided), average_emission_(float3(-1.f)) {}

Emissionmap::~Emissionmap() = default;

material::Sample const& Emissionmap::sample(float3 const&      wo, Ray const& /*ray*/,
                                            Renderstate const& rs, Filter       filter,
                                            Sampler& /*sampler*/, Worker const& worker) const {
    auto& sample = worker.sample<Sample>();

    auto& sampler = worker.sampler_2D(sampler_key(), filter);

    sample.set_basis(rs.geo_n, wo);

    float3 const radiance = emission_map_.sample_3(worker, sampler, rs.uv);

    sample.set(emission_factor_ * radiance);

    return sample;
}

float3 Emissionmap::evaluate_radiance(float3 const& /*wi*/, float2 uv, float /*area*/,
                                      Filter filter, Worker const& worker) const {
    auto& sampler = worker.sampler_2D(sampler_key(), filter);

    return emission_factor_ * emission_map_.sample_3(worker, sampler, uv);
}

float3 Emissionmap::average_radiance(float /*area*/, Scene const& /*scene*/) const {
    return average_emission_;
}

float Emissionmap::ior() const {
    return 1.5f;
}

bool Emissionmap::has_emission_map() const {
    return emission_map_.is_valid();
}

Material::Sample_2D Emissionmap::radiance_sample(float2 r2) const {
    auto const result = distribution_.sample_continuous(r2);

    return {result.uv, result.pdf * total_weight_};
}

float Emissionmap::emission_pdf(float2 uv, Filter filter, Worker const& worker) const {
    auto& sampler = worker.sampler_2D(sampler_key(), filter);

    return distribution_.pdf(sampler.address(uv)) * total_weight_;
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
}

void Emissionmap::set_emission_factor(float emission_factor) {
    emission_factor_ = emission_factor;
}

size_t Emissionmap::num_bytes() const {
    return sizeof(*this) + distribution_.num_bytes();
}

void Emissionmap::prepare_sampling_internal(Shape const& shape, int32_t element,
                                            bool importance_sampling, thread::Pool& threads,
                                            Scene const& scene) {
    auto const& texture = emission_map_.texture(scene);

    if (importance_sampling) {
        auto const d = texture.dimensions_2();

        Distribution_2D::Distribution_impl* conditional = distribution_.allocate(uint32_t(d[1]));

        memory::Array<float4> artws(threads.num_threads(), float4(0.f));

        float2 const idf = 1.f / float2(d);

        float const ef = emission_factor_;

        threads.run_range(
            [conditional, &artws, &shape, &texture, d, idf, element, ef](uint32_t id, int32_t begin,
                                                                         int32_t end) noexcept {
                auto luminance = memory::Buffer<float>(uint32_t(d[0]));

                float4 artw(0.f);

                for (int32_t y = begin; y < end; ++y) {
                    float const v = idf[1] * (float(y) + 0.5f);

                    for (int32_t x = 0; x < d[0]; ++x) {
                        float const u = idf[0] * (float(x) + 0.5f);

                        float const uv_weight = shape.uv_weight(float2(u, v));

                        float3 const radiance = ef * texture.at_element_3(x, y, element);

                        float3 const wr = uv_weight * radiance;

                        artw += float4(wr, uv_weight);

                        luminance[x] = spectrum::luminance(wr);
                    }

                    conditional[y].init(luminance.data(), uint32_t(d[0]));
                }

                artws[id] += artw;
            },
            0, d[1]);

        float4 artw(0.f);
        for (auto const& a : artws) {
            artw += a;
        }

        average_emission_ = artw.xyz() / artw[3];

        total_weight_ = artw[3];

        distribution_.init();
    } else {
        average_emission_ = emission_factor_ * texture.average_3();
    }

    if (is_two_sided()) {
        average_emission_ *= 2.f;
    }
}

}  // namespace scene::material::light
