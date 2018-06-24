#include "photon_mapper.hpp"
#include "photon_map.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/light/light.hpp"
#include "scene/light/light_sample.hpp"
#include "scene/prop/prop_intersection.inl"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"

#include <iostream>

namespace rendering::integrator::photon {

Mapper::Mapper(rnd::Generator& rng, take::Settings const& settings)
    : Integrator(rng, settings), sampler_(rng) {}

Mapper::~Mapper() {}

void Mapper::prepare(Scene const& /*scene*/, uint32_t num_photons) {
    sampler_.resize(num_photons, 1, 1, 1);
}

void Mapper::resume_pixel(uint32_t /*sample*/, rnd::Generator& /*scramble*/) {}

uint32_t Mapper::bake(Map& map, int2 range, Worker& worker) {
    uint32_t num_paths = 0;

    for (int32_t i = range[0]; i < range[1]; ++i) {
        Photon         photon;
        uint32_t const num_iterations = trace_photon(worker, photon);

        if (num_iterations > 0) {
            map.insert(photon, i);
            num_paths += num_iterations;
        } else {
            std::cout << "sad" << std::endl;
        }
    }

    return num_paths;
}

size_t Mapper::num_bytes() const {
    return sizeof(*this);
}

uint32_t Mapper::trace_photon(Worker& worker, Photon& photon) {
    static constexpr uint32_t Max_iterations = 100;

    Intersection intersection;

    for (uint32_t i = 0; i < Max_iterations; ++i) {
        Ray    ray;
        float3 radiance;
        if (!generate_light_ray(worker, ray, radiance)) {
            continue;
        }

        //     float const ray_offset = take_settings_.ray_offset_factor * intersection.geo.epsilon;

        if (!worker.intersect_and_resolve_mask(ray, intersection, Sampler_filter::Nearest)) {
            continue;
        }

        photon.p     = intersection.geo.p;
        photon.wi    = -ray.direction;
        photon.alpha = radiance;

        return i + 1;
    }

    return 0;
}

bool Mapper::generate_light_ray(Worker& worker, Ray& ray, float3& radiance) {
    float const select = sampler_.generate_sample_1D(1);

    auto const light = worker.scene().random_light(select);

    scene::light::Sample_from light_sample;
    if (!light.ref.sample(0.f, sampler_, 0, Sampler_filter::Nearest, worker, light_sample)) {
        return false;
    }

    ray.origin = light_sample.shape.p;
    ray.set_direction(light_sample.shape.dir);
    ray.min_t      = take_settings_.ray_offset_factor * light_sample.shape.epsilon;
    ray.max_t      = scene::Ray_max_t;
    ray.time       = 0.f;
    ray.wavelength = 0.f;

    radiance = light_sample.radiance / (light.pdf * light_sample.shape.pdf);

    return true;
}

}  // namespace rendering::integrator::photon
