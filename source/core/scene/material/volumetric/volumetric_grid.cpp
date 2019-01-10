#include "volumetric_grid.hpp"
#include "base/math/distribution/distribution_1d.inl"
#include "base/math/matrix4x4.inl"
#include "base/math/ray.inl"
#include "base/random/generator.inl"
#include "base/spectrum/heatmap.hpp"
#include "base/spectrum/rgb.hpp"
#include "base/thread/thread_pool.hpp"
#include "image/texture/texture_adapter.inl"
#include "scene/entity/composed_transformation.hpp"
#include "scene/material/collision_coefficients.inl"
#include "scene/scene_worker.hpp"
#include "volumetric_octree_builder.hpp"

namespace scene::material::volumetric {

Grid::Grid(Sampler_settings const& sampler_settings, Texture_adapter const& density) noexcept
    : Material(sampler_settings), density_(density) {}

Grid::~Grid() noexcept {}

float3 Grid::evaluate_radiance(float3 const& /*wi*/, float3 const& uvw, float /*volume*/,
                               Filter filter, Worker const& worker) const noexcept {
    float const d = density(uvw, filter, worker);

    return d * cc_.a * emission_;
}

CC Grid::collision_coefficients() const noexcept {
    return cc_;
}

CC Grid::collision_coefficients(float2 /*uv*/, Filter /*filter*/, Worker const& /*worker*/) const
    noexcept {
    return cc_;
}

CC Grid::collision_coefficients(float3 const& uvw, Filter filter, Worker const& worker) const
    noexcept {
    float const d = density(uvw, filter, worker);

    return {d * cc_.a, d * cc_.s};
}

CCE Grid::collision_coefficients_emission(float3 const& uvw, Filter filter,
                                          Worker const& worker) const noexcept {
    float const d = density(uvw, filter, worker);

    return {{d * cc_.a, d * cc_.s}, emission_};
}

void Grid::compile(thread::Pool& pool) noexcept {
    auto const& texture = density_.texture();

    Octree_builder builder;
    builder.build(tree_, texture, cm_, pool);
}

Gridtree const* Grid::volume_tree() const noexcept {
    return &tree_;
}

bool Grid::is_heterogeneous_volume() const noexcept {
    return true;
}

size_t Grid::num_bytes() const noexcept {
    return sizeof(*this) + tree_.num_bytes();
}

float Grid::density(float3 const& uvw, Filter filter, Worker const& worker) const noexcept {
    auto const& sampler = worker.sampler_3D(sampler_key(), filter);

    return density_.sample_1(sampler, uvw);
}

Grid_emission::Grid_emission(Sampler_settings const& sampler_settings,
                             Texture_adapter const&  grid) noexcept
    : Grid(sampler_settings, grid), average_emission_(float3(-1.f)) {}

Grid_emission::~Grid_emission() noexcept {}

bool Grid_emission::has_emission_map() const noexcept {
    return true;
}

Grid_emission::Sample_3D Grid_emission::radiance_sample(float3 const& r3) const noexcept {
    auto const result = distribution_.sample_continuous(r3);

    return {result.uvw, result.pdf * total_weight_};
}

float Grid_emission::emission_pdf(float3 const& uvw, Filter filter, Worker const& worker) const
    noexcept {
    auto& sampler = worker.sampler_3D(sampler_key(), filter);

    return distribution_.pdf(sampler.address(uvw)) * total_weight_;
}

void Grid_emission::prepare_sampling(Shape const& /*shape*/, uint32_t /*part*/, uint64_t /*time*/,
                                     Transformation const& /*transformation*/, float /*area*/,
                                     bool importance_sampling, thread::Pool& pool) noexcept {
    if (average_emission_[0] >= 0.f) {
        // Hacky way to check whether prepare_sampling has been called before
        // average_emission_ is initialized with negative values...
        return;
    }

    auto const& texture = density_.texture();

    float3 const emission = cc_.a * emission_;

    if (importance_sampling) {
        auto const& d = texture.dimensions_3();

        Distribution_2D* conditional_2d = distribution_.allocate(d[2]);

        std::vector<float3> ars(pool.num_threads(), float3(0.f));

        pool.run_range(
            [&emission, &conditional_2d, &ars, &texture, d](uint32_t id, int32_t begin,
                                                            int32_t end) {
                float* luminance = memory::allocate_aligned<float>(d[0]);

                float3 ar(0.f);

                for (int32_t z = begin; z < end; ++z) {
                    Distribution_2D::Distribution_impl* conditional = conditional_2d[z].allocate(
                        d[1]);

                    for (int32_t y = 0; y < d[1]; ++y) {
                        for (int32_t x = 0; x < d[0]; ++x) {
                            float const density = texture.at_1(x, y, z);

                            float3 const radiance = density * emission;

                            luminance[x] = spectrum::luminance(radiance);

                            ar += radiance;
                        }

                        conditional[y].init(luminance, d[0]);
                    }

                    conditional_2d[z].init();
                }

                ars[id] += ar;

                memory::free_aligned(luminance);
            },
            0, d[2]);

        float3 ar(0.f);
        for (auto const& a : ars) {
            ar += a;
        }

        float const total_weight = static_cast<float>(d[0] * d[1] * d[2]);

        average_emission_ = ar / total_weight;

        total_weight_ = total_weight;

        distribution_.init();
    } else {
        average_emission_ = texture.average_1() * emission;
    }
}

size_t Grid_emission::num_bytes() const noexcept {
    return sizeof(*this) + tree_.num_bytes() + distribution_.num_bytes();
}

Grid_color::Grid_color(Sampler_settings const& sampler_settings,
                       Texture_adapter const&  color) noexcept
    : Material(sampler_settings), color_(color) {}

Grid_color::~Grid_color() noexcept {}

float3 Grid_color::evaluate_radiance(float3 const& /*wi*/, float3 const& uvw, float /*volume*/,
                                     Filter filter, Worker const& worker) const noexcept {
    float3 const c = color(uvw, filter, worker);

    CC const cc = attenuation(c, distance_);

    return cc.a * emission_;
}

CC Grid_color::collision_coefficients() const noexcept {
    return cc_;
}

CC Grid_color::collision_coefficients(float2 /*uv*/, Filter /*filter*/,
                                      Worker const& /*worker*/) const noexcept {
    return cc_;
}

CC Grid_color::collision_coefficients(float3 const& uvw, Filter filter, Worker const& worker) const
    noexcept {
    float3 const c = color(uvw, filter, worker);

    return attenuation(c, distance_);
}

CCE Grid_color::collision_coefficients_emission(float3 const& uvw, Filter filter,
                                                Worker const& worker) const noexcept {
    float3 const c = color(uvw, filter, worker);

    CC const cc = attenuation(c, distance_);

    return {cc, emission_};
}

void Grid_color::compile(thread::Pool& pool) noexcept {
    auto const& texture = color_.texture();

    Octree_builder builder;
    builder.build(tree_, texture, CM(distance_), pool);
}

Gridtree const* Grid_color::volume_tree() const noexcept {
    return &tree_;
}

bool Grid_color::is_heterogeneous_volume() const noexcept {
    return true;
}

size_t Grid_color::num_bytes() const noexcept {
    return sizeof(*this) + tree_.num_bytes();
}

float3 Grid_color::color(float3 const& uvw, Filter filter, Worker const& worker) const noexcept {
    auto const& sampler = worker.sampler_3D(sampler_key(), filter);

    return color_.sample_3(sampler, uvw);
}

}  // namespace scene::material::volumetric
