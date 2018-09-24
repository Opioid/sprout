#include "photon_mapper.hpp"
#include "base/math/aabb.inl"
#include "photon_map.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/light/light.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_sample.inl"
#include "scene/prop/interface_stack.inl"
#include "scene/prop/prop_intersection.inl"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/shape/shape_sample.hpp"

namespace rendering::integrator::photon {

using namespace scene;

Mapper::Mapper(rnd::Generator& rng, take::Settings const& take_settings,
               Settings const& settings) noexcept
    : Integrator(rng, take_settings),
      settings_(settings),
      sampler_(rng),
      photons_(memory::allocate_aligned<Photon>(settings.max_bounces)) {}

Mapper::~Mapper() noexcept {
    memory::free_aligned(photons_);
}

void Mapper::prepare(Scene const& /*scene*/, uint32_t /*num_photons*/) noexcept {
    sampler_.resize(1, 1, 1, 1);
}

void Mapper::resume_pixel(uint32_t /*sample*/, rnd::Generator& /*scramble*/) noexcept {}

uint32_t Mapper::bake(Map& map, int32_t begin, int32_t end, float normalized_tick_offset,
                      float normalized_tick_slice, Worker& worker) noexcept {
    math::AABB const& bounds = settings_.full_light_path ? worker.scene().aabb()
                                                         : worker.scene().caustic_aabb();

    bool const infinite_world = worker.scene().is_infinite();

    uint32_t num_paths = 0;

    for (int32_t i = begin; i < end; ++i) {
        uint32_t const max_photons = std::min(settings_.max_bounces,
                                              static_cast<uint32_t>(end - i));

        uint32_t       num_photons;
        uint32_t const num_iterations = trace_photon(normalized_tick_offset, normalized_tick_slice,
                                                     bounds, infinite_world, worker, max_photons,
                                                     photons_, num_photons);

        if (num_iterations > 0) {
            for (uint32_t j = 0; j < num_photons; ++j) {
                map.insert(photons_[j], static_cast<uint32_t>(i) + j);
            }

            i += num_photons - 1;

            num_paths += num_iterations;
        } else {
            return 0;
        }
    }

    return num_paths;
}

size_t Mapper::num_bytes() const noexcept {
    return sizeof(*this);
}

uint32_t Mapper::trace_photon(float normalized_tick_offset, float normalized_tick_slice,
                              math::AABB const& bounds, bool infinite_world, Worker& worker,
                              uint32_t max_photons, Photon* photons,
                              uint32_t& num_photons) noexcept {
    // How often should we try to create a valid photon path?
    static uint32_t constexpr Max_iterations = 1024 * 10;

    math::AABB unnatural_limit = bounds;
    unnatural_limit.scale(8.f);

    Sampler_filter const filter = Sampler_filter::Undefined;

    bool const avoid_caustics = false;

    Bxdf_sample sample_result;

    Intersection intersection;

    uint32_t iteration = 0;

    num_photons = 0;

    for (uint32_t i = 0; i < Max_iterations; ++i) {
        worker.interface_stack().clear();

        bool specular_ray = false;

        Ray                 ray;
        light::Light const* light;
        shape::Sample_from  light_sample;
        if (!generate_light_ray(normalized_tick_offset, normalized_tick_slice, bounds, worker, ray,
                                &light, light_sample)) {
            continue;
        }

        if (!worker.intersect_and_resolve_mask(ray, intersection, filter)) {
            continue;
        }

        float3 radiance = light->evaluate(light_sample, 0.f, Sampler_filter::Nearest, worker) /
                          (light_sample.pdf);

        for (; ray.depth < settings_.max_bounces;) {
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

            float const ray_offset = take_settings_.ray_offset_factor * intersection.geo.epsilon;

            if (material_sample.ior_greater_one()) {
                bool const singular = sample_result.type.test_any(Bxdf_type::Specular,
                                                                  Bxdf_type::Transmission);

                if (singular) {
                    specular_ray = true;
                } else if ((intersection.subsurface || material_sample.same_hemisphere(wo)) &&
                           ((specular_ray &&
                             worker.interface_stack().top_is_vacuum_or_pure_specular()) ||
                            settings_.full_light_path)) {
                    if (infinite_world && !unnatural_limit.intersect(intersection.geo.p)) {
                        break;
                    }

                    auto& photon = photons[num_photons];

                    photon.p        = intersection.geo.p;
                    photon.wi       = wo;
                    photon.alpha[0] = radiance[0];
                    photon.alpha[1] = radiance[1];
                    photon.alpha[2] = radiance[2];
                    photon.properties.set(Photon::Property::First_hit, 0 == num_photons);
                    photon.properties.set(Photon::Property::Volumetric, intersection.subsurface);

                    iteration = i + 1;

                    ++num_photons;

                    if (max_photons == num_photons) {
                        return iteration;
                    }

                    specular_ray = settings_.indirect_caustics;
                } else if (!settings_.indirect_caustics) {
                    break;
                }

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

            if (0.f == ray.wavelength) {
                ray.wavelength = sample_result.wavelength;
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

bool Mapper::generate_light_ray(float normalized_tick_offset, float normalized_tick_slice,
                                math::AABB const& bounds, Worker& worker, Ray& ray,
                                light::Light const** light_out,
                                shape::Sample_from&  light_sample) noexcept {
    float const select = sampler_.generate_sample_1D(1);

    auto const light = worker.scene().random_light(select);

    if (!light.ref.sample(0.f, sampler_, 0, bounds, worker, light_sample)) {
        return false;
    }

    ray.origin = light_sample.p;
    ray.set_direction(light_sample.dir);
    ray.min_t = take_settings_.ray_offset_factor * light_sample.epsilon;
    ray.max_t = scene::Ray_max_t;
    ray.depth = 0;
    ray.time  = 0.f;
    ray.time  = normalized_tick_offset + sampler_.generate_sample_1D(2) * normalized_tick_slice;
    ray.wavelength = 0.f;

    *light_out = &light.ref;

    light_sample.pdf *= light.pdf;

    return true;
}

}  // namespace rendering::integrator::photon
