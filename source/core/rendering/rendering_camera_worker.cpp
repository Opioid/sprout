#include "rendering_camera_worker.hpp"
#include "rendering/integrator/surface/surface_integrator.hpp"
#include "rendering/sensor/sensor.hpp"
#include "sampler/camera_sample.hpp"
#include "sampler/sampler.hpp"
#include "scene/scene_ray.inl"
#include "scene/camera/camera.hpp"
#include "base/math/vector.inl"
#include "base/math/sampling/sample_distribution.inl"
#include "base/random/generator.inl"

namespace rendering {

void Camera_worker::render(scene::camera::Camera& camera, uint32_t view,
						   const int4& tile, uint32_t sample_begin, uint32_t sample_end,
						   float normalized_tick_offset, float normalized_tick_slice) {
	auto& sensor = camera.sensor();

	int4 bounds = camera.view_bounds(view);

	int4 view_tile(bounds.xy() + tile.xy(), bounds.xy() + tile.zw());

	sampler::Camera_sample sample;
	scene::Ray ray;

	rnd::Generator rng(tile.v[0] + 2, tile.v[1] + 8, tile.v[2] + 16, tile.v[3] + 128);

	for (int32_t y = tile.v[1], y_len = tile.v[3] + 1; y < y_len; ++y) {
		for (int32_t x = tile.v[0], x_len = tile.v[2] + 1; x < x_len; ++x) {
			sampler_->resume_pixel(sample_begin, rng);
			surface_integrator_->resume_pixel(sample_begin, rng);

			int2 pixel(x, y);

			for (uint32_t i = sample_begin; i < sample_end; ++i) {
				sampler_->generate_camera_sample(pixel, i, sample);

				sample.time = normalized_tick_offset + sample.time * normalized_tick_slice;

				if (camera.generate_ray(sample, view, ray)) {
					float4 color = li(ray);
					sensor.add_sample(sample, color, view_tile, bounds);
				} else {
					sensor.add_sample(sample, float4(0.f), view_tile, bounds);
				}
			}
		}
	}
}

}
