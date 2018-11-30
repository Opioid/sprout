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

}  // namespace scene::material::volumetric
