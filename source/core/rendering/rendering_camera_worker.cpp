#include "rendering_camera_worker.hpp"
#include "base/math/vector4.inl"
#include "base/random/generator.inl"
#include "rendering/integrator/surface/surface_integrator.hpp"
#include "rendering/integrator/volume/volume_integrator.hpp"
#include "rendering/sensor/sensor.hpp"
#include "rendering/tile_queue.hpp"
#include "sampler/camera_sample.hpp"
#include "sampler/sampler.hpp"
#include "scene/camera/camera.hpp"
#include "scene/scene_ray.inl"

namespace rendering {

Camera_worker::Camera_worker(Tile_queue const& tiles) : tiles_(tiles) {}

void Camera_worker::render(uint32_t view, int4 const& tile,
                           uint32_t sample_begin, uint32_t sample_end, float normalized_tick_offset,
                           float normalized_tick_slice) noexcept {
    scene::camera::Camera const& camera = *camera_;

    auto& sensor = camera.sensor();

    int4 bounds = camera.view_bounds(view);
    bounds[2] -= bounds[0];
    bounds[3] -= bounds[1];

    int4 isolated_bounds = sensor.isolated_tile(
        int4(bounds.xy() + tile.xy(), bounds.xy() + tile.zw()));
    isolated_bounds[2] -= isolated_bounds[0];
    isolated_bounds[3] -= isolated_bounds[1];

    uint32_t const tile_index = tiles_.index(tile);

    if (0 == sample_begin) {
        rng_.start(0, tile_index);
    } else {
        rng_.set_state(tiles_.random_state(tile_index));
    }

    uint64_t const sequence = 0xFFFFFFFFFFFFFFFF - static_cast<uint64_t>(tile_index);

    rnd::Generator scramble_rng(0, sequence);

    for (int32_t y = tile[1], y_len = tile[3] + 1; y < y_len; ++y) {
        for (int32_t x = tile[0], x_len = tile[2] + 1; x < x_len; ++x) {
            sampler_->resume_pixel(sample_begin, scramble_rng);
            surface_integrator_->resume_pixel(sample_begin, scramble_rng);

            int2 const pixel(x, y);

            for (uint32_t i = sample_begin; i < sample_end; ++i) {
                sampler::Camera_sample sample = sampler_->generate_camera_sample(pixel, i);

                sample.time = normalized_tick_offset + sample.time * normalized_tick_slice;

                if (Ray ray; camera.generate_ray(sample, view, ray)) {
                    float4 const color = li(ray, camera.interface_stack());
                    sensor.add_sample(sample, color, isolated_bounds, bounds);
                } else {
                    sensor.add_sample(sample, float4(0.f), isolated_bounds, bounds);
                }
            }
        }
    }

    tiles_.set_random_state(tile_index, rng_.state());
}

}  // namespace rendering
