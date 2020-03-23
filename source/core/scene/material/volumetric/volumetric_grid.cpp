#include "volumetric_grid.hpp"
#include "base/math/distribution/distribution_1d.inl"
#include "base/math/matrix4x4.inl"
#include "base/math/ray.inl"
#include "base/memory/array.inl"
#include "base/random/generator.inl"
#include "base/spectrum/mapping.hpp"
#include "base/spectrum/rgb.hpp"
#include "base/thread/thread_pool.hpp"
#include "image/texture/texture.inl"
#include "image/texture/texture_adapter.inl"
#include "scene/entity/composed_transformation.hpp"
#include "scene/material/collision_coefficients.inl"
#include "scene/scene_worker.hpp"
#include "volumetric_octree_builder.hpp"

namespace scene::material::volumetric {

Grid::Grid(Sampler_settings const& sampler_settings, Texture_adapter const& density)
    : Material(sampler_settings), density_(density) {
    properties_.set(Property::Heterogeneous_volume);
}

Grid::~Grid() = default;

float3 Grid::evaluate_radiance(float3 const& /*wi*/, float3 const& uvw, float /*volume*/,
                               Filter filter, Worker const& worker) const {
    float const d = density(uvw, filter, worker);

    return d * cc_.a * emission_;
}

CC Grid::collision_coefficients(float3 const& uvw, Filter filter, Worker const& worker) const {
    float const d = density(uvw, filter, worker);

    return {d * cc_.a, d * cc_.s};
}

CCE Grid::collision_coefficients_emission(float3 const& uvw, Filter filter,
                                          Worker const& worker) const {
    float const d = density(uvw, filter, worker);

    return {{d * cc_.a, d * cc_.s}, emission_};
}

void Grid::commit(thread::Pool& threads, Scene const& scene) {
    auto const& texture = density_.texture(scene);

    Octree_builder builder;
    builder.build(tree_, texture, cm_, threads);

    properties_.set(Property::Scattering_volume, any_greater_zero(cc_.s));
}

Gridtree const* Grid::volume_tree() const {
    return &tree_;
}

size_t Grid::num_bytes() const {
    return sizeof(*this) + tree_.num_bytes();
}

float Grid::density(float3 const& uvw, Filter filter, Worker const& worker) const {
    auto const& sampler = worker.sampler_3D(sampler_key(), filter);

    return density_.sample_1(worker, sampler, uvw);
}

Grid_emission::Grid_emission(Sampler_settings const& sampler_settings, Texture_adapter const& grid)
    : Grid(sampler_settings, grid), average_emission_(float3(-1.f)) {
    properties_.set(Property::Emission_map);
}

Grid_emission::~Grid_emission() = default;

Grid_emission::Sample_3D Grid_emission::radiance_sample(float3 const& r3) const {
    auto const result = distribution_.sample_continuous(r3);

    return {result.uvw, result.pdf * total_weight_};
}

float Grid_emission::emission_pdf(float3 const& uvw, Filter filter, Worker const& worker) const {
    auto& sampler = worker.sampler_3D(sampler_key(), filter);

    return distribution_.pdf(sampler.address(uvw)) * total_weight_;
}

void Grid_emission::prepare_sampling(Shape const& /*shape*/, uint32_t /*part*/, uint64_t /*time*/,
                                     Transformation const& /*transformation*/, float /*area*/,
                                     bool importance_sampling, thread::Pool& threads,
                                     Scene const& scene) {
    if (average_emission_[0] >= 0.f) {
        // Hacky way to check whether prepare_sampling has been called before
        // average_emission_ is initialized with negative values...
        return;
    }

    auto const& texture = density_.texture(scene);

    float3 const emission = cc_.a * emission_;

    if (importance_sampling) {
        auto const& d = texture.dimensions();

        Distribution_2D* conditional_2d = distribution_.allocate(uint32_t(d[2]));

        memory::Array<float3> ars(threads.num_threads());

        threads.run_range(
            [&emission, &conditional_2d, &ars, &texture, d](uint32_t id, int32_t begin,
                                                            int32_t end) noexcept {
                auto luminance = memory::Buffer<float>(uint32_t(d[0]));

                float3 ar(0.f);

                for (int32_t z = begin; z < end; ++z) {
                    auto conditional = conditional_2d[z].allocate(uint32_t(d[1]));

                    for (int32_t y = 0; y < d[1]; ++y) {
                        for (int32_t x = 0; x < d[0]; ++x) {
                            float const density = texture.at_1(x, y, z);

                            float3 const radiance = density * emission;

                            luminance[x] = spectrum::luminance(radiance);

                            ar += radiance;
                        }

                        conditional[y].init(luminance, uint32_t(d[0]));
                    }

                    conditional_2d[z].init();
                }

                ars[id] = ar;
            },
            0, d[2]);

        float3 ar(0.f);
        for (auto const& a : ars) {
            ar += a;
        }

        float const total_weight = float(d[0] * d[1] * d[2]);

        average_emission_ = ar / total_weight;

        total_weight_ = total_weight;

        distribution_.init();
    } else {
        average_emission_ = texture.average_1() * emission;
    }
}

size_t Grid_emission::num_bytes() const {
    return sizeof(*this) + tree_.num_bytes() + distribution_.num_bytes();
}

Grid_color::Grid_color(Sampler_settings const& sampler_settings) : Material(sampler_settings) {
    properties_.set(Property::Heterogeneous_volume);
    cc_ = CC{float3(0.5f), float3(0.5f)};
    cm_ = CM(cc_);
}

Grid_color::~Grid_color() = default;

void Grid_color::set_color(Texture_adapter const& color) {
    color_ = color;
}

float3 Grid_color::evaluate_radiance(float3 const& /*wi*/, float3 const& uvw, float /*volume*/,
                                     Filter filter, Worker const& worker) const {
    float4 const c = color(uvw, filter, worker);

    CC const cc = c[3] * attenuation(c.xyz(), scattering_factor_ * c.xyz(), attenuation_distance_);

    return cc.a * emission_;
}

CC Grid_color::collision_coefficients(float3 const& uvw, Filter filter,
                                      Worker const& worker) const {
    float4 const c = color(uvw, filter, worker);

    return c[3] * attenuation(c.xyz(), scattering_factor_ * c.xyz(), attenuation_distance_);
}

CCE Grid_color::collision_coefficients_emission(float3 const& uvw, Filter filter,
                                                Worker const& worker) const {
    float4 const c = color(uvw, filter, worker);

    CC const cc = c[3] * attenuation(c.xyz(), scattering_factor_ * c.xyz(), attenuation_distance_);

    return {cc, emission_};
}

void Grid_color::set_attenuation(float scattering_factor, float distance) {
    attenuation_distance_ = distance;
    scattering_factor_    = scattering_factor;
}

void Grid_color::commit(thread::Pool& threads, Scene const& scene) {
    auto const& texture = color_.texture(scene);

    Octree_builder builder;
    builder.build(tree_, texture, CM(attenuation_distance_, scattering_factor_), threads);
}

Gridtree const* Grid_color::volume_tree() const {
    return &tree_;
}

size_t Grid_color::num_bytes() const {
    return sizeof(*this) + tree_.num_bytes();
}

float4 Grid_color::color(float3 const& uvw, Filter filter, Worker const& worker) const {
    auto const& sampler = worker.sampler_3D(sampler_key(), filter);

    return color_.sample_4(worker, sampler, uvw);
}

}  // namespace scene::material::volumetric
