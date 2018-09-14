#include "aurora_provider.hpp"
#include "aurora.hpp"
#include "aurora_spectrum.hpp"
#include "base/json/json.hpp"
#include "base/math/matrix4x4.inl"
#include "base/math/quaternion.inl"
#include "base/math/sample_distribution.inl"
#include "base/math/sampling.inl"
#include "base/math/vector3.inl"
#include "base/random/generator.inl"
#include "base/thread/thread_pool.hpp"
#include "core/image/texture/texture_adapter.inl"
#include "core/image/texture/texture_byte_3_srgb.hpp"
#include "core/image/typed_image.inl"
#include "core/resource/resource_manager.hpp"
#include "core/scene/material/volumetric/volumetric_grid.hpp"
#include "core/scene/prop/prop.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_loader.hpp"
#include "volume_filter.hpp"
#include "volume_rasterizer.hpp"

#include <iostream>

namespace procedural::aurora {

using namespace scene;
using namespace scene::material;

Provider provider;

void init(scene::Loader& loader) {
    provider.set_scene_loader(loader);

    loader.register_extension_provider("Aurora", &provider);
}

void Provider::set_scene_loader(Loader& loader) {
    scene_loader_ = &loader;
}

entity::Entity* Provider::create_extension(json::Value const& /*extension_value*/, Scene& scene,
                                           resource::Manager& manager) {
    spectrum::init();

    using namespace image;

    // const int3 dimensions(2048, 512, 256);
    // const int3 dimensions(3072, 384, 192);
    //	const int3 dimensions(3200, 320, 160);
    //	const int3 dimensions(3200, 320, 256);

    //	const int3 dimensions(1600, 160, 128);
    const int3 dimensions(1600, 128, 102);

    //	const int3 dimensions(320, 32, 16);

    //	const int3 dimensions(32, 32, 32);

    auto target = std::make_shared<Byte3>(Image::Description(Image::Type::Byte3, dimensions));

    render(*target, manager.thread_pool());

    auto texture = std::make_shared<texture::Byte3_sRGB>(target);

    std::shared_ptr<Material> material = std::make_shared<volumetric::Emission_grid>(
        Sampler_settings(), Texture_adapter(texture));

    Materials materials(1);
    materials[0] = material;

    prop::Prop* volume = scene.create_prop(scene_loader_->cube(), materials);
    // volume::Volume* volume = scene.create_height_volume();

    math::Transformation transformation{float3::identity(),
                                        //	float3(1000000.f, 100000.f, 50000.f),
                                        float3(1250000.f, 100000.f, 80000.f),
                                        //	float3(10.f, 1.f, 1.f),
                                        math::quaternion::identity()};

    volume->set_transformation(transformation);

    Aurora* aurora = new Aurora();

    aurora->attach(volume);

    return aurora;
}

void Provider::render(image::Byte3& target, thread::Pool& thread_pool) {
    rnd::Generator rng;

    Volume_rasterizer renderer(target.description().dimensions);

    float3 const dimensions(target.description().dimensions);

    voxel_ratio_ = dimensions[1] / aurora_height_;

    aurora_volume_ = float3(dimensions[0] / voxel_ratio_, aurora_height_,
                            dimensions[2] / voxel_ratio_);

    renderer.set_brush(float3(0.f));
    renderer.clear();

    float const start_height_deviation = 60000.f;
    float const start_height = ground_to_bottom_ + aurora_height_ - start_height_deviation;

    uint32_t const num_particles = 4 * 16384;

    for (uint32_t i = 0; i < num_particles; ++i) {
        float const x = math::radical_inverse_vdC(i, 0);

        //	float const ix = 1.f - x;

        float const z = std::sin(x * (2.f * math::Pi)) * 50000.f +
                        std::sin(3.5f * x * (2.f * math::Pi)) * 20000.f +
                        (x * x) * std::sin(17.f * x * (2.f * math::Pi)) * 12000.f;

        float const y = (start_height + std::sin(x * (7.f * math::Pi)) * start_height_deviation);
        //	float const y = start_height + start_height_deviation;

        float3 const position(x * aurora_volume_[0], ground_to_bottom_ + aurora_height_,
                              0.5f * aurora_volume_[2] + z);

        simulate_particle(position, y, rng, renderer);
    }

    // Keep those as useful
    //	float const filter_radius = 8.f;
    //	float const alpha = 0.04f;

    //	float const filter_radius = 4.f;
    //	float const alpha = 0.125f;

    float const filter_radius = 6.f;
    float const alpha         = 0.075f;

    Volume_filter filter(target.description().dimensions, filter_radius, alpha,
                         thread_pool.num_threads());

    //	filter.filter(renderer.data(), thread_pool);

    renderer.resolve(target);
}

void Provider::simulate_particle(float3 const& start, float peak_height, rnd::Generator& rng,
                                 Volume_rasterizer& renderer) const {
    float const step_size = 1.f / voxel_ratio_;

    float const range       = (start[1] - ground_to_bottom_);
    float const grace_range = (start[1] - peak_height);

    float3 position = start;
    for (float edge = ground_to_bottom_ + (8.f * step_size); position[1] > edge;) {
        float const normalized_height = (position[1] - ground_to_bottom_) / aurora_height_;

        float const progress       = 1.f - (position[1] - ground_to_bottom_) / range;
        float const grace_progress = std::min(1.f - (position[1] - peak_height) / grace_range, 1.f);

        //	float const progress = 1.f - normalized_height;
        float const spread = progress * progress * progress * 3000.f;

        float3 const color = progress * grace_progress * spectrum::linear_rgb(normalized_height);

        renderer.splat(world_to_grid(position), color);

        float2 const r2(rng.random_float(), rng.random_float());
        float2 const uv = math::sample_disk_concentric(r2);

        position[0] += uv[0] * spread;
        position[2] += uv[1] * spread;

        position[1] -= step_size;
    }
}

float3 Provider::world_to_grid(float3 const& position) const {
    return voxel_ratio_ *
           float3(position[0], (ground_to_bottom_ + aurora_height_) - position[1], position[2]);
}

}  // namespace procedural::aurora
