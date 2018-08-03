#include "light_emissionmap.hpp"
#include "base/math/distribution/distribution_2d.inl"
#include "base/math/math.hpp"
#include "base/math/vector4.inl"
#include "base/spectrum/rgb.hpp"
#include "base/thread/thread_pool.hpp"
#include "image/texture/texture_adapter.inl"
#include "light_material_sample.hpp"
#include "scene/material/material_sample.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"
#include "scene/shape/shape.hpp"

namespace scene::material::light {

Emissionmap::Emissionmap(Sampler_settings const& sampler_settings, bool two_sided)
    : Material(sampler_settings, two_sided), average_emission_(float3(-1.f)) {}

Emissionmap::~Emissionmap() {}

material::Sample const& Emissionmap::sample(float3 const& wo, Renderstate const& rs,
                                            Sampler_filter filter, sampler::Sampler& /*sampler*/,
                                            Worker const& worker, uint32_t depth) const {
    auto& sample = worker.sample<Sample>(depth);

    auto& sampler = worker.sampler_2D(sampler_key(), filter);

    sample.set_basis(rs.geo_n, wo);

    sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);

    float3 const radiance = emission_map_.sample_3(sampler, rs.uv);
    sample.layer_.set(emission_factor_ * radiance);

    return sample;
}

float3 Emissionmap::evaluate_radiance(float3 const& /*wi*/, float2   uv, float /*area*/,
                                      float /*time*/, Sampler_filter filter,
                                      Worker const& worker) const {
    auto& sampler = worker.sampler_2D(sampler_key(), filter);
    return emission_factor_ * emission_map_.sample_3(sampler, uv);
}

float3 Emissionmap::average_radiance(float /*area*/) const {
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

float Emissionmap::emission_pdf(float2 uv, Sampler_filter filter, Worker const& worker) const {
    auto& sampler = worker.sampler_2D(sampler_key(), filter);

    return distribution_.pdf(sampler.address(uv)) * total_weight_;
}

void Emissionmap::prepare_sampling(shape::Shape const& shape, uint32_t /*part*/,
                                   Transformation const& /*transformation*/, float /*area*/,
                                   bool importance_sampling, thread::Pool& pool) {
    if (average_emission_[0] >= 0.f) {
        // Hacky way to check whether prepare_sampling has been called before
        // average_emission_ is initialized with negative values...
        return;
    }

    if (importance_sampling) {
        auto const& texture = emission_map_.texture();

        auto const d = texture.dimensions_2();

        std::vector<math::Distribution_2D::Distribution_impl> conditional(d[1]);

        std::vector<float4> artws(pool.num_threads(), float4::identity());

        float2 const rd(1.f / static_cast<float>(d[0]), 1.f / static_cast<float>(d[1]));

        float const ef = emission_factor_;

        pool.run_range(
            [&conditional, &artws, &shape, &texture, d, rd, ef](uint32_t id, int32_t begin,
                                                                int32_t end) {
                std::vector<float> luminance(d[0]);

                float4 artw(0.f);

                for (int32_t y = begin; y < end; ++y) {
                    float const v = rd[1] * (static_cast<float>(y) + 0.5f);

                    for (int32_t x = 0; x < d[0]; ++x) {
                        float const u = rd[0] * (static_cast<float>(x) + 0.5f);

                        float const uv_weight = shape.uv_weight(float2(u, v));

                        float3 const radiance = ef * texture.at_3(x, y);

                        luminance[x] = uv_weight * spectrum::luminance(radiance);

                        artw += float4(uv_weight * radiance, uv_weight);
                    }

                    conditional[y].init(luminance.data(), d[0]);
                }

                artws[id] += artw;
            },
            0, d[1]);

        // artw: (float3(averave_radiance), total_weight)
        float4 artw(0.f);
        for (auto& a : artws) {
            artw += a;
        }

        average_emission_ = artw.xyz() / artw[3];

        total_weight_ = artw[3];

        distribution_.init(conditional);
    } else {
        average_emission_ = emission_factor_ * emission_map_.texture().average_3();
    }

    if (is_two_sided()) {
        average_emission_ *= 2.f;
    }
}

size_t Emissionmap::num_bytes() const {
    return sizeof(*this) + distribution_.num_bytes();
}

void Emissionmap::set_emission_map(Texture_adapter const& emission_map) {
    emission_map_ = emission_map;
}

void Emissionmap::set_emission_factor(float emission_factor) {
    emission_factor_ = emission_factor;
}

}  // namespace scene::material::light
