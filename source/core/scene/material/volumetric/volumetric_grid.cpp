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
#include "scene/scene_worker.hpp"
#include "volumetric_octree_builder.hpp"

namespace scene::material::volumetric {

Grid::Grid(Sampler_settings const& sampler_settings, Texture_adapter const& grid) noexcept
    : Material(sampler_settings), grid_(grid) {}

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

Grid_emission::Grid_emission(Sampler_settings const& sampler_settings,
                             Texture_adapter const&  grid) noexcept
    : Grid(sampler_settings, grid) {}

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

void Grid_emission::prepare_sampling(shape::Shape const& shape, uint32_t part, uint64_t time,
                                     Transformation const& transformation, float area,
                                     bool importance_sampling, thread::Pool& pool) noexcept {
    if (importance_sampling) {
        auto const& texture = grid_.texture();

        auto const& d = texture.dimensions_3();

        std::vector<math::Distribution_2D> conditional(static_cast<uint32_t>(d[2]));

        std::vector<float4> artws(pool.num_threads(), float4::identity());

        float3 const emission = cc_.a * emission_;

        pool.run_range(
            [&emission, &conditional, &artws, &texture, d](uint32_t id, int32_t begin,
                                                           int32_t end) {
                std::vector<float> luminance(static_cast<uint32_t>(d[0]));

                float4 artw(0.f);

                for (int32_t z = begin; z < end; ++z) {
                    std::vector<math::Distribution_2D::Distribution_impl> conditional_2d(
                        static_cast<uint32_t>(d[1]));

                    for (int32_t y = 0; y < d[1]; ++y) {
                        for (int32_t x = 0; x < d[0]; ++x) {
                            float const density = texture.at_1(x, y, z);

                            float3 const radiance = density * emission;

                            luminance[static_cast<uint32_t>(x)] = spectrum::luminance(radiance);

                            artw += float4(radiance, 1.f);
                        }

                        conditional_2d[static_cast<uint32_t>(y)].init(luminance.data(),
                                                                      static_cast<uint32_t>(d[0]));
                    }

                    conditional[static_cast<uint32_t>(z)].init(conditional_2d);
                }

                artws[id] += artw;
            },
            0, d[2]);

        float4 artw(0.f);
        for (auto& a : artws) {
            artw += a;
        }

        average_emission_ = artw.xyz() / artw[3];

        total_weight_ = artw[3];

        distribution_.init(conditional);
    } else {
        average_emission_ = grid_.texture().average_1() * cc_.a * emission_;
    }
}

}  // namespace scene::material::volumetric
