#include "volumetric_grid.hpp"
#include "base/math/matrix4x4.inl"
#include "base/math/ray.inl"
#include "base/random/generator.inl"
#include "base/spectrum/heatmap.hpp"
#include "image/texture/texture_adapter.inl"
#include "scene/entity/composed_transformation.hpp"
#include "scene/scene_worker.hpp"
#include "volumetric_octree_builder.hpp"

namespace scene::material::volumetric {

Grid::Grid(Sampler_settings const& sampler_settings, Texture_adapter const& grid) noexcept
    : Density(sampler_settings), grid_(grid) {}

Grid::~Grid() noexcept {}

void Grid::compile(thread::Pool& pool) noexcept {
    auto const& texture = grid_.texture();

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

    return grid_.sample_1(sampler, uvw);
}

Emission_grid::Emission_grid(Sampler_settings const& sampler_settings,
                             Texture_adapter const&  grid) noexcept
    : Material(sampler_settings), grid_(grid) {}

Emission_grid::~Emission_grid() noexcept {}

float3 Emission_grid::emission(math::Ray const& ray, Transformation const& transformation,
                               float step_size, rnd::Generator& rng, Filter filter,
                               Worker const& worker) const noexcept {
    math::Ray const rn = ray.normalized();

    float min_t = rn.min_t + rng.random_float() * step_size;

    float3 emission(0.f);

    float3 const rp_o = math::transform_point(transformation.world_to_object, rn.origin);
    float3 const rd_o = math::transform_vector(transformation.world_to_object, rn.direction);

    for (; min_t < rn.max_t; min_t += step_size) {
        float3 const p_o = rp_o + min_t * rd_o;  // r_o.point(min_t);
        emission += Emission_grid::emission(p_o, filter, worker);
    }

    return step_size * emission;
}

size_t Emission_grid::num_bytes() const noexcept {
    return sizeof(*this);
}

float3 Emission_grid::emission(float3 const& p, Filter filter, Worker const& worker) const
    noexcept {
    // p is in object space already

    float3 p_g = 0.5f * (float3(1.f) + p);
    p_g[1]     = 1.f - p_g[1];

    auto const& sampler = worker.sampler_3D(sampler_key(), filter);

    return 0.00001f * grid_.sample_3(sampler, p_g);
}

Flow_vis_grid::Flow_vis_grid(Sampler_settings const& sampler_settings,
                             Texture_adapter const&  grid) noexcept
    : Material(sampler_settings), grid_(grid) {}

Flow_vis_grid::~Flow_vis_grid() noexcept {}

float3 Flow_vis_grid::emission(math::Ray const& ray, Transformation const& transformation,
                               float step_size, rnd::Generator& rng, Filter filter,
                               Worker const& worker) const noexcept {
    math::Ray const rn = ray.normalized();

    float min_t = rn.min_t + rng.random_float() * step_size;

    float3 emission(0.f);

    float3 const rp_o = math::transform_point(transformation.world_to_object, rn.origin);
    float3 const rd_o = math::transform_vector(transformation.world_to_object, rn.direction);

    for (; min_t < rn.max_t; min_t += step_size) {
        float3 const p_o = rp_o + min_t * rd_o;  // r_o.point(min_t);
        emission += Flow_vis_grid::emission(p_o, filter, worker);
    }

    return step_size * 8.f * emission;

    /*
            float density = 0.f;

            float3 const rp_o = math::transform_point(rn.origin, transformation.world_to_object);
            float3 const rd_o = math::transform_vector(rn.direction,
       transformation.world_to_object);

            for (; min_t < rn.max_t; min_t += step_size) {
                    float3 const p_o = rp_o + min_t * rd_o; // r_o.point(min_t);
                    density += Flow_vis_grid::density(p_o, filter, worker);
            }

            density *= std::min(48.f * step_size, 1.f);

            return spectrum::heatmap(density);
            */
}

size_t Flow_vis_grid::num_bytes() const noexcept {
    return sizeof(*this);
}

float Flow_vis_grid::density(float3 const& p, Filter filter, Worker const& worker) const noexcept {
    // p is in object space already

    float3 p_g = 0.5f * (float3(1.f) + p);
    p_g[1]     = 1.f - p_g[1];

    auto const& sampler = worker.sampler_3D(sampler_key(), filter);

    return grid_.sample_1(sampler, p_g);
}

float3 Flow_vis_grid::emission(float3 const& p, Filter filter, Worker const& worker) const
    noexcept {
    // p is in object space already

    float3 p_g = 0.5f * (float3(1.f) + p);
    p_g[1]     = 1.f - p_g[1];

    auto const& sampler = worker.sampler_3D(sampler_key(), filter);

    float const density = std::min(2.f * grid_.sample_1(sampler, p_g), 1.f);

    return spectrum::heatmap(density);
}

}  // namespace scene::material::volumetric
