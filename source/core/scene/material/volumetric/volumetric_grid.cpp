#include "volumetric_grid.hpp"
#include "base/math/distribution_1d.inl"
#include "base/math/interpolated_function_1d.inl"
#include "base/math/matrix4x4.inl"
#include "base/math/ray.inl"
#include "base/memory/array.inl"
#include "base/memory/buffer.hpp"
#include "base/random/generator.inl"
#include "base/spectrum/mapping.hpp"
#include "base/spectrum/rgb.hpp"
#include "base/thread/thread_pool.hpp"
#include "image/texture/texture.inl"
#include "scene/composed_transformation.hpp"
#include "scene/material/collision_coefficients.inl"
#include "scene/material/material.inl"
#include "scene/scene_worker.inl"
#include "volumetric_octree_builder.hpp"

namespace scene::material::volumetric {

Grid::Grid(Sampler_settings sampler_settings, Texture const& density)
    : Material(sampler_settings), density_(density) {
    properties_.set(Property::Heterogeneous_volume);
}

Grid::~Grid() = default;

float3 Grid::evaluate_radiance(float3_p /*wi*/, float3_p /*n*/, float3_p uvw, float /*volume*/,
                               Filter filter, Worker const& worker) const {
    float const d = density(uvw, filter, worker);

    return d * cc_.a * emission_;
}

CC Grid::collision_coefficients(float3_p uvw, Filter filter, Worker const& worker) const {
    float const d = density(uvw, filter, worker);

    return {d * cc_.a, d * cc_.s};
}

CCE Grid::collision_coefficients_emission(float3_p uvw, Filter filter, Worker const& worker) const {
    float const d = density(uvw, filter, worker);

    return {{d * cc_.a, d * cc_.s}, emission_};
}

void Grid::commit(Threads& threads, Scene const& scene) {
    Octree_builder builder;
    builder.build(tree_, density_, &cc_, scene, threads);

    properties_.set(Property::Scattering_volume,
                    any_greater_zero(cc_.s) || any_greater_zero(emission_));
}

Gridtree const* Grid::volume_tree() const {
    return &tree_;
}

image::Description Grid::useful_texture_description(const Scene& scene) const {
    return density_.description(scene);
}

float Grid::density(float3_p uvw, Filter filter, Worker const& worker) const {
    auto const& sampler = worker.sampler_3D(sampler_key(), filter);

    return sampler.sample_1(density_, uvw, worker.scene());
}

Grid_emission::Grid_emission(Sampler_settings sampler_settings, Texture const& grid)
    : Grid(sampler_settings, grid), average_emission_(float3(-1.f)) {
    properties_.set(Property::Emission_map);
}

Grid_emission::~Grid_emission() = default;

float3 Grid_emission::evaluate_radiance(float3_p /*wi*/, float3_p /*n*/, float3_p uvw,
                                        float /*volume*/, Filter filter,
                                        Worker const& worker) const {
    auto const& sampler = worker.sampler_3D(sampler_key(), filter);

    float3 const emission = temperature_.is_valid()
                                ? blackbody_(sampler.sample_1(temperature_, uvw, worker.scene()))
                                : emission_;

    if (2 == density_.num_channels()) {
        float2 const d = sampler.sample_2(density_, uvw, worker.scene());

        return (d[0] * d[1]) * a_norm_ * emission;
    } else {
        float const d = sampler.sample_1(density_, uvw, worker.scene());

        return d * a_norm_ * emission;
    }
}

Grid_emission::Radiance_sample Grid_emission::radiance_sample(float3_p r3) const {
    auto const result = distribution_.sample_continuous(r3);

    return {result.xyz(), result[3] * pdf_factor_};
}

float Grid_emission::emission_pdf(float3_p uvw, Worker const& worker) const {
    auto& sampler = worker.sampler_3D(sampler_key(), Filter::Undefined);

    return distribution_.pdf(sampler.address(uvw)) * pdf_factor_;
}

CCE Grid_emission::collision_coefficients_emission(float3_p uvw, Filter filter,
                                                   Worker const& worker) const {
    auto const& sampler = worker.sampler_3D(sampler_key(), filter);

    float3 const emission = temperature_.is_valid()
                                ? blackbody_(sampler.sample_1(temperature_, uvw, worker.scene()))
                                : emission_;

    if (2 == density_.num_channels()) {
        float2 const d = sampler.sample_2(density_, uvw, worker.scene());

        return {{d[0] * cc_.a, d[0] * cc_.s}, d[1] * emission};
    } else {
        float const d = sampler.sample_1(density_, uvw, worker.scene());

        return {{d * cc_.a, d * cc_.s}, emission};
    }
}

void Grid_emission::commit(Threads& threads, Scene const& scene) {
    average_emission_ = float3(-1.f);

    Octree_builder builder;

    if (1 == density_.num_channels()) {
        builder.build(tree_, density_, &cc_, scene, threads);
    } else {
        CC const ccs[] = {cc_, CC{cc_.a, float3(0.f)}};

        builder.build(tree_, density_, ccs, scene, threads);
    }

    properties_.set(Property::Scattering_volume,
                    any_greater_zero(cc_.s) || any_greater_zero(emission_));

    if (temperature_.is_valid()) {
        static uint32_t constexpr Num_samples = 16;

        // fire
        //        static float constexpr Start = 1000.f;
        //        static float constexpr End   = 5400.f;

        //        blackbody_.allocate(0.f, 1.6f, Num_samples);

        // explosion
        static float constexpr Start = 2000.f;
        static float constexpr End   = 5000.f;

        blackbody_.allocate(0.f, 1.2f, Num_samples);

        for (uint32_t i = 0; i < Num_samples; ++i) {
            float const t = Start + float(i) / float(Num_samples - 1) * (End - Start);

            float3 const c = spectrum::blackbody(t);

            blackbody_[i] = emission_ * c;
        }
    }

    //    auto const& tt = density_.texture(scene);

    //    float max_t = 0.f;

    //    for (int32_t z = 0; z < tt.dimensions()[2]; ++z) {
    //        for (int32_t y = 0; y < tt.dimensions()[1]; ++y) {
    //            for (int32_t x = 0; x < tt.dimensions()[0]; ++x) {
    //                max_t = std::max(max_t, tt.at_1(x, y, z));
    //            }
    //        }
    //    }

    //    std::cout << max_t << std::endl;
}

float3 Grid_emission::prepare_sampling(Shape const& /*shape*/, uint32_t /*part*/,
                                       Transformation const& /*trafo*/, float /*area*/,
                                       Scene const& scene, Threads& threads) {
    if (average_emission_[0] >= 0.f) {
        // Hacky way to check whether prepare_sampling has been called before
        // average_emission_ is initialized with negative values...

        return average_emission_;
    }

    auto const& d = density_.description(scene).dimensions();

    Distribution_2D* conditional_2d = distribution_.allocate(uint32_t(d[2]));

    memory::Array<float3> ars(threads.num_threads());

    memory::Buffer<float> luminance(d[0] * d[1] * d[2]);

    if (temperature_.is_valid()) {
        threads.run_range(
            [this, &luminance, &ars, d, &scene](uint32_t id, int32_t begin, int32_t end) noexcept {
                float3 ar(0.f);

                if (2 == density_.num_channels()) {
                    for (int32_t z = begin; z < end; ++z) {
                        int32_t const slice = z * (d[0] * d[1]);

                        for (int32_t y = 0; y < d[1]; ++y) {
                            int32_t const row = y * d[0];
                            for (int32_t x = 0; x < d[0]; ++x) {
                                float2 const density = density_.at_2(x, y, z, scene);

                                float const t = temperature_.at_1(x, y, z, scene);

                                float3 const c = blackbody_(t);

                                float3 const radiance = density[0] * density[1] * c;

                                luminance[slice + row + x] = spectrum::luminance(radiance);

                                ar += radiance;
                            }
                        }
                    }
                } else {
                    for (int32_t z = begin; z < end; ++z) {
                        int32_t const slice = z * (d[0] * d[1]);

                        for (int32_t y = 0; y < d[1]; ++y) {
                            int32_t const row = y * d[0];
                            for (int32_t x = 0; x < d[0]; ++x) {
                                float const density = density_.at_1(x, y, z, scene);

                                float const t = temperature_.at_1(x, y, z, scene);

                                float3 const c = blackbody_(t);

                                float3 const radiance = density * c;

                                luminance[slice + row + x] = spectrum::luminance(radiance);

                                ar += radiance;
                            }
                        }
                    }
                }

                ars[id] = ar;
            },
            0, d[2]);

    } else {
        float3 const emission = emission_;

        threads.run_range(
            [this, &emission, &luminance, &ars, d, &scene](uint32_t id, int32_t begin,
                                                           int32_t end) noexcept {
                float3 ar(0.f);

                for (int32_t z = begin; z < end; ++z) {
                    int32_t const slice = z * (d[0] * d[1]);

                    for (int32_t y = 0; y < d[1]; ++y) {
                        int32_t const row = y * d[0];
                        for (int32_t x = 0; x < d[0]; ++x) {
                            float const density = density_.at_1(x, y, z, scene);

                            float3 const radiance = density * emission;

                            luminance[slice + row + x] = spectrum::luminance(radiance);

                            ar += radiance;
                        }
                    }
                }

                ars[id] = ar;
            },
            0, d[2]);
    }

    float3 ar(0.f);
    for (auto const& a : ars) {
        ar += a;
    }

    float const num_pixels = float(d[0] * d[1] * d[2]);

    float3 const average_emission = ar / num_pixels;

    float const al = 0.6f * spectrum::luminance(average_emission);

    threads.run_range(
        [&luminance, &conditional_2d, al, d](uint32_t /*id*/, int32_t begin, int32_t end) noexcept {
            for (int32_t z = begin; z < end; ++z) {
                auto conditional = conditional_2d[z].allocate(uint32_t(d[1]));

                int32_t const slice = z * (d[0] * d[1]);

                for (int32_t y = 0; y < d[1]; ++y) {
                    float* luminance_row = luminance.data() + (slice + y * d[0]);

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

                conditional_2d[z].init();
            }
        },
        0, d[2]);

    distribution_.init();

    average_emission_ = average_emission;

    float const majorant_a = max_component(cc_.a);

    a_norm_ = majorant_a / cc_.a;

    pdf_factor_ = num_pixels / majorant_a;

    return average_emission_;
}

void Grid_emission::set_temperature_map(Texture const& temperature_map) {
    temperature_ = temperature_map;
}

Grid_color::Grid_color(Sampler_settings sampler_settings) : Material(sampler_settings) {
    properties_.set(Property::Heterogeneous_volume);
    cc_ = CC{float3(0.5f), float3(0.5f)};
}

Grid_color::~Grid_color() = default;

void Grid_color::set_color(Texture const& color) {
    color_ = color;
}

float3 Grid_color::evaluate_radiance(float3_p /*wi*/, float3_p /*n*/, float3_p uvw,
                                     float /*volume*/, Filter filter, Worker const& worker) const {
    float4 const c = color(uvw, filter, worker);

    CC const cc = c[3] * attenuation(c.xyz(), scattering_factor_ * c.xyz(), attenuation_distance_,
                                     volumetric_anisotropy_);

    return cc.a * emission_;
}

CC Grid_color::collision_coefficients(float3_p uvw, Filter filter, Worker const& worker) const {
    float4 const c = color(uvw, filter, worker);

    return c[3] * attenuation(c.xyz(), scattering_factor_ * c.xyz(), attenuation_distance_,
                              volumetric_anisotropy_);
}

CCE Grid_color::collision_coefficients_emission(float3_p uvw, Filter filter,
                                                Worker const& worker) const {
    float4 const c = color(uvw, filter, worker);

    CC const cc = c[3] * attenuation(c.xyz(), scattering_factor_ * c.xyz(), attenuation_distance_,
                                     volumetric_anisotropy_);

    return {cc, emission_};
}

void Grid_color::set_volumetric(float scattering_factor, float distance, float anisotropy) {
    attenuation_distance_  = distance;
    scattering_factor_     = scattering_factor;
    volumetric_anisotropy_ = anisotropy;
}

void Grid_color::commit(Threads& threads, Scene const& scene) {
    CC const hack{float3(attenuation_distance_), float3(scattering_factor_)};

    Octree_builder builder;
    builder.build(tree_, color_, &hack, scene, threads);
}

Gridtree const* Grid_color::volume_tree() const {
    return &tree_;
}

float4 Grid_color::color(float3_p uvw, Filter filter, Worker const& worker) const {
    auto const& sampler = worker.sampler_3D(sampler_key(), filter);

    return sampler.sample_4(color_, uvw, worker.scene());
}

}  // namespace scene::material::volumetric
