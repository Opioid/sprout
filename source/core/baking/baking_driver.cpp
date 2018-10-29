#include "baking_driver.hpp"
#include <fstream>
#include "baking_worker.hpp"
#include "base/math/sampling.inl"
#include "base/math/vector4.inl"
#include "base/random/generator.inl"
#include "image/encoding/png/png_writer.hpp"
#include "image/typed_image.inl"
#include "rendering/integrator/surface/surface_integrator.hpp"
#include "rendering/integrator/volume/volume_integrator.hpp"
#include "sampler/sampler.hpp"
#include "scene/scene.hpp"
#include "scene/scene_ray.inl"

namespace baking {

Driver::Driver(std::shared_ptr<Surface_integrator_factory> surface_integrator_factory,
               std::shared_ptr<Volume_integrator_factory>  volume_integrator_factory,
               std::shared_ptr<sampler::Factory>           sampler_factory)
    : surface_integrator_factory_(surface_integrator_factory),
      volume_integrator_factory_(volume_integrator_factory),
      sampler_factory_(sampler_factory) {}

void Driver::render(scene::Scene& /*scene*/, const take::View& /*view*/,
                    thread::Pool& /*thread_pool*/, uint32_t /*max_sample_size*/,
                    exporting::Sink& /*exporter*/, progress::Sink& /*progressor*/) {
    /*	int2 dimensions(512, 512);

            scene.tick(thread_pool);

            image::Float4 target(image::Image::Description(image::Image::Type::Float4, dimensions));

            baking::Baking_worker worker;
            worker.init(0,  scene, max_sample_size, *surface_integrator_factory_,
                                    *volume_integrator_factory_, *sampler_factory_);

            scene::Ray ray;
            ray.time = 0.f;

            float3 bake_quad_origin(-2.f, 0.f, 2.f);
            float3 bake_quad_extent(2.f, 0.f, -2.f);
            float3 bake_quad_range = bake_quad_extent - bake_quad_origin;

            uint32_t num_samples = 4096;

            float3 bake_space_x(1.f, 0.f, 0.f);
            float3 bake_space_y(0.f, 0.f, -1.f);
            float3 bake_space_z(0.f, 1.f, 0.f);

            for (int32_t y = 0; y < dimensions[1]; ++y) {
                    for (int32_t x = 0; x < dimensions[0]; ++x) {
                    //	worker.sampler()->start_pixel(0, rng);

                            float3 offset((static_cast<float>(x) + 0.5f) *
                                                       (bake_quad_range[0] /
       static_cast<float>(dimensions[0])), 0.f, (static_cast<float>(y) + 0.5f) * (bake_quad_range[2]
       / static_cast<float>(dimensions[1])));

                            float3 origin = bake_quad_origin + offset;

                            float3 irradiance = float3::identity();

                            for (uint32_t s = 0; s < num_samples; ++s) {
                                    ray.origin = origin;

                                    float2 sample = worker.sampler()->generate_sample_2D();
                                    float3 hs = math::sample_oriented_hemisphere_cosine(sample,
                                                                                                                                            bake_space_x,
                                                                                                                                            bake_space_y,
                                                                                                                                            bake_space_z);

                                    ray.set_direction(hs);
                                    ray.depth = 1;
                                    ray.min_t = 0.01f;
                                    ray.max_t = 10000.f;

                                    irradiance += worker.li(ray).xyz();
                            }

                            irradiance /= static_cast<float>(num_samples);

                            auto& pixel = target.at(x, y);

                            pixel[0] = irradiance[0];
                            pixel[1] = irradiance[1];
                            pixel[2] = irradiance[2];
                            pixel[3] = 1.f;
                    }
            }

            image::encoding::png::Writer writer(dimensions);

            std::ofstream stream("pastry.png", std::ios::binary);

            writer.write(stream, target, thread_pool);*/
}

}  // namespace baking
