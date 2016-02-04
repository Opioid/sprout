#include "rendering_camera_worker.hpp"
#include "rendering/integrator/surface/surface_integrator.hpp"
#include "rendering/sensor/sensor.hpp"
#include "sampler/camera_sample.hpp"
#include "sampler/sampler.hpp"
#include "scene/scene_ray.inl"
#include "scene/camera/camera.hpp"
#include "base/math/vector.inl"

namespace rendering {

void Camera_worker::render(scene::camera::Camera& camera, uint32_t view, const math::Recti& tile,
						   uint32_t sample_begin, uint32_t sample_end,
						   float normalized_tick_offset, float normalized_tick_slice) {
	auto& sensor = camera.sensor();

	math::Recti bounds = camera.view_bounds(view);

	uint32_t num_samples = sample_end - sample_begin;

	sampler::Camera_sample sample;
	scene::Ray ray;

	for (int32_t y = tile.start.y; y < tile.end.y; ++y) {
		for (int32_t x = tile.start.x; x < tile.end.x; ++x) {
			math::int2 pixel(x, y);

			if (0 == sample_begin) {
				if (0 == view) {
					math::uint2 seed = sampler_->seed();
					sampler_->set_seed(seed);
					camera.set_seed(pixel, seed);
				} else {
					sampler_->set_seed(camera.seed(pixel));
				}

				sampler_->restart(1);

				surface_integrator_->start_new_pixel(num_samples);
			} else {
				sampler_->set_seed(camera.seed(pixel));
			}

			for (uint32_t i = sample_begin; i < sample_end; ++i) {
				sampler_->generate_camera_sample(pixel, i, sample);

				ray.time = normalized_tick_offset + sample.time * normalized_tick_slice;;

				camera.generate_ray(sample, view, ray);

				math::float4 color = li(ray);

				sensor.add_sample(sample, color, tile, bounds);
			}
		}
	}
}

}
