#include "baking_driver.hpp"
#include "baking_worker.hpp"
#include "image/typed_image.inl"
#include "image/encoding/png/png_writer.hpp"
#include "rendering/integrator/surface/surface_integrator.hpp"
#include "rendering/integrator/volume/volume_integrator.hpp"
#include "sampler/sampler.hpp"
#include "scene/scene.hpp"
#include "scene/scene_ray.inl"
#include "base/math/random/generator.inl"
#include "base/math/sampling/sampling.inl"
#include "base/math/matrix.inl"
#include "base/math/vector.inl"
#include <fstream>

namespace baking {

Driver::Driver(std::shared_ptr<rendering::integrator::surface::Integrator_factory> surface_integrator_factory,
			   std::shared_ptr<rendering::integrator::volume::Integrator_factory> volume_integrator_factory,
			   std::shared_ptr<sampler::Sampler> sampler) :
	surface_integrator_factory_(surface_integrator_factory),
	volume_integrator_factory_(volume_integrator_factory),
	sampler_(sampler) {}

void Driver::render(scene::Scene& scene, const take::View& /*view*/, thread::Pool& thread_pool,
					exporting::Sink& /*exporter*/, progress::Sink& /*progressor*/) {
	math::int2 dimensions(512, 512);

	scene.tick(thread_pool);

	image::Image_float_4 target(image::Image::Description(image::Image::Type::Float_4, dimensions));

	math::random::Generator rng(0, 1, 2, 3);
	baking::Baking_worker worker;
	worker.init(0, rng, *surface_integrator_factory_, *volume_integrator_factory_, *sampler_, scene);

	scene::Ray ray;
	ray.time = 0.f;

	math::float3 bake_quad_origin(-2.f, 0.f, 2.f);
	math::float3 bake_quad_extent(2.f, 0.f, -2.f);
	math::float3 bake_quad_range = bake_quad_extent - bake_quad_origin;

	uint32_t num_samples = 4096;

	math::float3 bake_space_x(1.f, 0.f, 0.f);
	math::float3 bake_space_y(0.f, 0.f, -1.f);
	math::float3 bake_space_z(0.f, 1.f, 0.f);

	for (int32_t y = 0; y < dimensions.y; ++y) {
		for (int32_t x = 0; x < dimensions.x; ++x) {

			sampler_->restart_and_seed(num_samples);

			math::float3 offset((static_cast<float>(x) + 0.5f) * (bake_quad_range.x / static_cast<float>(dimensions.x)),
								0.f,
								(static_cast<float>(y) + 0.5f) * (bake_quad_range.z / static_cast<float>(dimensions.y)));

			math::float3 origin = bake_quad_origin + offset;

			math::float3 irradiance = math::float3_identity;

			for (uint32_t s = 0; s < num_samples; ++s) {

				ray.origin = origin;

				math::float2 sample = sampler_->generate_sample_2D();
				math::float3 hs = math::sample_oriented_hemisphere_cosine(sample, bake_space_x, bake_space_y, bake_space_z);

				ray.set_direction(hs);
				ray.depth = 1;
				ray.min_t = 0.01f;
				ray.max_t = 10000.f;

				irradiance += worker.li(ray).xyz;
			}

			irradiance /= static_cast<float>(num_samples);

			auto& pixel = target.at(x, y);

			pixel.x = irradiance.x;
			pixel.y = irradiance.y;
			pixel.z = irradiance.z;
			pixel.w = 1.f;
		}
	}

	image::encoding::png::Writer writer(dimensions);

	std::ofstream stream("pastry.png", std::ios::binary);

	writer.write(stream, target, thread_pool);
}

}
