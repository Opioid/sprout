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

	int4 bounds = camera.view_bounds(view);
	bounds[2] -= bounds[0];
	bounds[3] -= bounds[1];

	int4 isolated_bounds = sensor.isolated_tile(int4(bounds.xy() + tile.xy(),
													 bounds.xy() + tile.zw()));
	isolated_bounds[2] -= isolated_bounds[0];
	isolated_bounds[3] -= isolated_bounds[1];

	sampler::Camera_sample sample;
	scene::Ray ray;

	// Actually, we just need a unique number per tile here that >= #workers.
	// Maybe we can come up with a more elegant expression sometime...
	const uint64_t sequence = 0xFFFFFFFFFFFFFFFF - ((static_cast<uint64_t>(tile[2]) << 32) +
													 static_cast<uint64_t>(tile[3]));
	rnd::Generator rng(0, sequence);

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
					sensor.add_sample(sample, color, isolated_bounds, bounds);
				} else {
					sensor.add_sample(sample, float4(0.f), isolated_bounds, bounds);
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
