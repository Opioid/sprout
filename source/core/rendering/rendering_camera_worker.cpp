#include "rendering_camera_worker.hpp"
#include "rendering/integrator/surface/surface_integrator.hpp"
#include "rendering/integrator/volume/volume_integrator.hpp"
#include "rendering/sensor/sensor.hpp"
#include "sampler/camera_sample.hpp"
#include "sampler/sampler.hpp"
#include "scene/camera/camera.hpp"
#include "scene/scene_ray.inl"
#include "base/math/vector4.inl"
#include "base/math/sampling/sample_distribution.hpp"
#include "base/random/generator.inl"

namespace rendering {

void Camera_worker::render(scene::camera::Camera& camera, uint32_t view,
						   const int4& tile, uint32_t sample_begin, uint32_t sample_end,
						   float normalized_tick_offset, float normalized_tick_slice) {
	auto& sensor = camera.sensor();

	const int4 bounds = camera.view_bounds(view);

	const int4 view_tile(bounds.xy() + tile.xy(), bounds.xy() + tile.zw());

	sampler::Camera_sample sample;
	scene::Ray ray;

	rnd::Generator rng(tile[0] + 2, tile[1] + 8, tile[2] + 16, tile[3] + 128);

	for (int32_t y = tile[1], y_len = tile[3] + 1; y < y_len; ++y) {
		for (int32_t x = tile[0], x_len = tile[2] + 1; x < x_len; ++x) {
			sampler_->resume_pixel(sample_begin, rng);
			surface_integrator_->resume_pixel(sample_begin, rng);

			const int2 pixel(x, y);

			for (uint32_t i = sample_begin; i < sample_end; ++i) {
				sampler_->generate_camera_sample(pixel, i, sample);

				sample.time = normalized_tick_offset + sample.time * normalized_tick_slice;

				if (camera.generate_ray(sample, view, ray)) {
					const float4 color = li(ray);
					sensor.add_sample(sample, color, view_tile, bounds);
				} else {
					sensor.add_sample(sample, float4(0.f), view_tile, bounds);
				}
			}
		}
	}
}

size_t Camera_worker::num_bytes() const {
	return sizeof(*this) +
		   surface_integrator_->num_bytes() +
		   volume_integrator_->num_bytes() +
		   sampler_->num_bytes();
}

}
