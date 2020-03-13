#include "rendering_camera_worker.hpp"
#include "base/math/vector4.inl"
#include "base/random/generator.inl"
#include "rendering/integrator/particle/lighttracer.hpp"
#include "rendering/integrator/surface/surface_integrator.hpp"
#include "rendering/integrator/volume/volume_integrator.hpp"
#include "rendering/sensor/sensor.hpp"
#include "rendering/tile_queue.hpp"
#include "rendering_worker.inl"
#include "sampler/camera_sample.hpp"
#include "sampler/sampler.hpp"
#include "scene/camera/camera.hpp"
#include "scene/scene.hpp"
#include "scene/scene_ray.inl"

namespace rendering {

Camera_worker::Camera_worker(uint32_t max_sample_size,
                             Range_queue const& ranges)
    : Worker(max_sample_size), ranges_(ranges) {}

void Camera_worker::render(uint32_t frame, uint32_t view, uint32_t iteration, int4 const& tile,
                           uint32_t num_samples) {
    Camera const& camera = *camera_;

    int2 const offset = camera.view_offset(view);

    int4 crop = camera.crop();
    crop[2] -= crop[0] + 1;
    crop[3] -= crop[1] + 1;
    crop[0] += offset[0];
    crop[1] += offset[1];

    int4 const view_tile(offset + tile.xy(), offset + tile.zw());

    auto& sensor = camera.sensor();

    int4 isolated_bounds = sensor.isolated_tile(view_tile);
    isolated_bounds[2] -= isolated_bounds[0];
    isolated_bounds[3] -= isolated_bounds[1];

    int2 const r = camera.resolution();

    uint64_t const so = uint64_t(iteration) * uint64_t(r[0] * r[1]);

    for (int32_t y = tile[1], y_back = tile[3]; y <= y_back; ++y) {
        for (int32_t x = tile[0], x_back = tile[2]; x <= x_back; ++x) {
            rng_.start(0, uint64_t(y * r[0] + x) + so);

            sampler_->start_pixel();
            surface_integrator_->start_pixel();
            volume_integrator_->start_pixel();

            int2 const pixel(x, y);

            for (uint32_t i = num_samples; i > 0; --i) {
                sampler::Camera_sample const sample = sampler_->generate_camera_sample(pixel);

                if (Ray ray; camera.generate_ray(sample, frame, view, *scene_, ray)) {
                    float4 const color = li(ray, camera.interface_stack());
                    sensor.add_sample(sample, color, isolated_bounds, offset, crop);
                } else {
                    sensor.add_sample(sample, float4(0.f), isolated_bounds, offset, crop);
                }
            }
        }
    }
}

void Camera_worker::particles(uint32_t frame, uint32_t iteration, uint32_t segment,
                              ulong2 const& range) {
    scene::camera::Camera const& camera = *camera_;

    uint32_t const range_index = ranges_.index(range, segment);

    rng_.start(0, range_index + iteration * ranges_.size());

    lighttracer_->start_pixel();

    for (uint64_t i = range[0]; i < range[1]; ++i) {
        particle_li(frame, camera.interface_stack());
    }
}

}  // namespace rendering
