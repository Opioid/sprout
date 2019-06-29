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
#include "scene/scene.hpp"
#include "scene/scene_ray.inl"

namespace rendering {

Camera_worker::Camera_worker(Tile_queue const& tiles) : tiles_(tiles) {}

void Camera_worker::render(uint32_t frame, uint32_t view, int4 const& tile,
                           uint32_t num_samples) noexcept {
    scene::camera::Camera const& camera = *camera_;

    int4 bounds = camera.view_bounds(view);
    bounds[2] -= bounds[0];
    bounds[3] -= bounds[1];

    auto& sensor = camera.sensor();

    int4 isolated_bounds = sensor.isolated_tile(
        int4(bounds.xy() + tile.xy(), bounds.xy() + tile.zw()));
    isolated_bounds[2] -= isolated_bounds[0];
    isolated_bounds[3] -= isolated_bounds[1];

    uint32_t const tile_index = tiles_.index(tile);

    rng_.start(0, tile_index);

    scene::prop::Prop const* camera_prop = scene_->prop(camera.entity());

    for (int32_t y = tile[1], y_len = tile[3] + 1; y < y_len; ++y) {
        for (int32_t x = tile[0], x_len = tile[2] + 1; x < x_len; ++x) {
            sampler_->start_pixel();
            surface_integrator_->start_pixel();

            int2 const pixel(x, y);

            for (uint32_t i = 0; i < num_samples; ++i) {
                sampler::Camera_sample const sample = sampler_->generate_camera_sample(pixel, i);

                if (Ray ray; camera.generate_ray(camera_prop, sample, frame, view, *scene_, ray)) {
                    float4 const color = li(ray, camera.interface_stack());
                    sensor.add_sample(sample, color, isolated_bounds, bounds);
                } else {
                    sensor.add_sample(sample, float4(0.f), isolated_bounds, bounds);
                }
            }
        }
    }
}

void Camera_worker::particles(uint32_t frame, uint32_t view) noexcept {
    scene::camera::Camera const& camera = *camera_;

    int4 bounds = camera.view_bounds(view);
    bounds[2] -= bounds[0];
    bounds[3] -= bounds[1];

    particle_li(bounds, camera.interface_stack());
}

}  // namespace rendering
