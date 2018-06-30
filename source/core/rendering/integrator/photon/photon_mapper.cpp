#include "photon_mapper.hpp"
#include "photon_map.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/light/light.hpp"
#include "scene/light/light_sample.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_sample.inl"
#include "scene/prop/interface_stack.inl"
#include "scene/prop/prop_intersection.inl"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"

#include <iostream>

namespace rendering::integrator::photon {

Mapper::Mapper(rnd::Generator& rng, take::Settings const& take_settings, Settings const& settings)
    : Integrator(rng, take_settings),
      settings_(settings),
      sampler_(rng),
      photons_(memory::allocate_aligned<Photon>(settings.max_bounces)) {}

Mapper::~Mapper() {
    memory::free_aligned(photons_);
}

void Mapper::prepare(Scene const& /*scene*/, uint32_t /*num_photons*/) {
    sampler_.resize(1, 1, 1, 1);
}

void Mapper::resume_pixel(uint32_t /*sample*/, rnd::Generator& /*scramble*/) {}

uint32_t Mapper::bake(Map& map, int32_t begin, int32_t end, Worker& worker) {
    uint32_t num_paths = 0;

    for (int32_t i = begin; i < end; ++i) {
        uint32_t const max_photons = std::min(settings_.max_bounces,
                                              static_cast<uint32_t>(end - i));
        uint32_t       num_photons;
        uint32_t const num_iterations = trace_photon(worker, max_photons, photons_, num_photons);

        if (num_iterations > 0) {
            for (uint32_t j = 0; j < num_photons; ++j) {
                map.insert(photons_[j], i + j);
            }

            i += num_photons - 1;

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

uint32_t Mapper::trace_photon(Worker& worker, uint32_t max_photons, Photon* photons,
                              uint32_t& num_photons) {
    static constexpr uint32_t Max_iterations = 2048;

    Sampler_filter const filter = Sampler_filter::Undefined;

    const bool avoid_caustics = false;

    Bxdf_sample sample_result;

    Intersection intersection;

    uint32_t iteration = 0;

    num_photons = 0;

    for (uint32_t i = 0; i < Max_iterations; ++i) {
        worker.interface_stack().clear();

        bool specular_ray = false;

        Ray    ray;
        float3 radiance;
        if (!generate_light_ray(worker, ray, radiance)) {
            continue;
        }

        if (!worker.intersect_and_resolve_mask(ray, intersection, filter)) {
            continue;
        }

        for (uint32_t j = settings_.max_bounces; j > 0; --j) {
            float3 const wo = -ray.direction;

            auto const& material_sample = intersection.sample(wo, ray, filter, avoid_caustics,
                                                              sampler_, worker);

            if (material_sample.is_pure_emissive()) {
                break;
            }

            material_sample.sample(sampler_, sample_result);
            if (0.f == sample_result.pdf) {
                break;
            }

            bool const singular = sample_result.type.test_any(Bxdf_type::Specular,
                                                              Bxdf_type::Transmission);

            if (singular && !settings_.full_light_path) {
                specular_ray = true;
            } else if ((specular_ray &&
                        worker.interface_stack().top_is_vacuum_or_pure_specular()) ||
                       settings_.full_light_path) {
                auto& photon = photons[num_photons];

                photon.p = intersection.geo.p;
                //                photon.n     = intersection.geo.n;
                photon.wi      = -ray.direction;
                photon.alpha   = radiance;
                photon.caustic = 0 == num_photons;

                iteration = i + 1;

                ++num_photons;

                if (max_photons == num_photons) {
                    return iteration;
                }

                specular_ray = settings_.indirect_caustics;
            } else if (!settings_.indirect_caustics) {
                break;
            }

            float const ray_offset = take_settings_.ray_offset_factor * intersection.geo.epsilon;

            if (material_sample.ior_greater_one()) {
                //  radiance *= sample_result.reflection / sample_result.pdf;

                float3 const nr      = radiance * sample_result.reflection / sample_result.pdf;
                float const  average = math::average(nr) / math::average(radiance);
                float const  continue_prob = std::min(1.f, average);

                if (sampler_.generate_sample_1D() > continue_prob) {
                    break;
                }

                radiance = nr / continue_prob;

                ray.origin = intersection.geo.p;
                ray.set_direction(sample_result.wi);
                ray.min_t = ray_offset;
                ++ray.depth;
            } else {
                ray.min_t = ray.max_t + ray_offset;
            }

            ray.max_t = scene::Ray_max_t;

            if (sample_result.type.test(Bxdf_type::Transmission)) {
                worker.interface_change(sample_result.wi, intersection);
            }

            if (!worker.interface_stack().empty()) {
                float3     vli, vtr;
                bool const hit = worker.volume(ray, intersection, filter, vli, vtr);

                //   radiance += throughput * vli;
                radiance *= vtr;

                if (!hit) {
                    break;
                }
            } else if (!worker.intersect_and_resolve_mask(ray, intersection, filter)) {
                break;
            }
        }

        if (iteration) {
            return iteration;
        }
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
    ray.depth      = 0;
    ray.time       = 0.f;
    ray.wavelength = 0.f;

    radiance = light_sample.radiance / (light.pdf * light_sample.shape.pdf);

    return true;
}

}  // namespace rendering::integrator::photon
