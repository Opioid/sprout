#include "photon_mapper.hpp"
#include "base/math/aabb.inl"
#include "base/math/frustum.hpp"
#include "base/memory/align.hpp"
#include "image/encoding/png/png_writer.hpp"
#include "photon.hpp"
#include "photon_map.hpp"
#include "rendering/integrator/integrator_helper.hpp"
#include "rendering/integrator/particle/particle_importance.hpp"
#include "rendering/rendering_worker.inl"
#include "scene/camera/camera.hpp"
#include "scene/light/light.inl"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_sample.inl"
#include "scene/prop/interface_stack.inl"
#include "scene/prop/prop_intersection.inl"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/shape/shape_sample.hpp"

//#define ISLAND_MODE

namespace rendering::integrator::particle::photon {

Mapper::Mapper(rnd::Generator& rng, Settings const& settings) noexcept
    : Integrator(rng),
      settings_(settings),
      sampler_(rng),
      photons_(memory::allocate_aligned<Photon>(settings.max_bounces)) {}

Mapper::~Mapper() noexcept {
    memory::free_aligned(photons_);
}

void Mapper::prepare(Scene const& /*scene*/, uint32_t /*num_photons*/) noexcept {
    sampler_.resize(1, 1, 1, 1);
}

void Mapper::start_pixel() noexcept {}

uint32_t Mapper::bake(Map& map, int32_t begin, int32_t end, uint32_t frame, uint32_t /*iteration*/,
                      Worker& worker) noexcept {
    Frustum const frustum = worker.camera().frustum();

    AABB const& world_bounds = settings_.full_light_path ? worker.scene().aabb()
                                                         : worker.scene().caustic_aabb();

    AABB const bounds = world_bounds.intersection(frustum.calculate_aabb());

    bool const infinite_world = worker.scene().is_infinite();

    bool const caustics_only = map.caustics_only();

    uint32_t num_paths = 0;

    for (int32_t i = begin; i < end;) {
        uint32_t const max_photons = std::min(settings_.max_bounces, uint32_t(end - i));

        uint32_t       num_photons;
        uint32_t       light_id;
        Sample_from    light_sample;
        uint32_t const num_iterations = trace_photon(frame, bounds, frustum, infinite_world,
                                                     caustics_only, worker, max_photons, photons_,
                                                     num_photons, light_id, light_sample);

        if (num_iterations > 0) {
            for (uint32_t j = 0; j < num_photons; ++j) {
                map.insert(photons_[j], uint32_t(i) + j);

                worker.particle_importance().increment(light_id, light_sample.xy, photons_[j].p);
            }

            i += num_photons;

            num_paths += num_iterations;

        } else {
            return 0;
        }
    }

    return num_paths;
}

uint32_t Mapper::trace_photon(uint32_t frame, AABB const& bounds, Frustum const& /*frustum*/,
                              bool infinite_world, bool caustics_only, Worker& worker,
                              uint32_t max_photons, Photon* photons, uint32_t& num_photons,
                              uint32_t& light_id, Sample_from& light_sample) noexcept {
    // How often should we try to create a valid photon path?
    static uint32_t constexpr Max_iterations = 1024 * 10;

    AABB unnatural_limit = bounds;
    unnatural_limit.scale(8.f);

    Filter const filter = Filter::Undefined;

    bool const avoid_caustics = false;

    Bxdf_sample sample_result;

    Intersection intersection;

    uint32_t iteration = 0;

    num_photons = 0;

    for (uint32_t i = 0; i < Max_iterations; ++i) {
        worker.interface_stack().clear();

        bool caustic_path = false;

        Ray ray;
        //   Light const* light;
        Light light;
        if (!generate_light_ray(frame, bounds, worker, ray, light, light_id, light_sample)) {
            continue;
        }

        if (!worker.intersect_and_resolve_mask(ray, intersection, filter)) {
            continue;
        }

        float3 radiance = light.evaluate(light_sample, Filter::Nearest, worker) /
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

#ifdef ISLAND_MODE
            if (0 == ray.depth && sample_result.type.no(Bxdf_type::Transmission)) {
                break;
            }
#endif

            if (material_sample.ior_greater_one()) {
                if (sample_result.type.is(Bxdf_type::Caustic)) {
                    caustic_path = true;
                } else {
                    if ((intersection.subsurface || material_sample.same_hemisphere(wo)) &&
                        ((caustic_path &&
                          worker.interface_stack().top_is_vacuum_or_not_scattering(worker)) ||
                         settings_.full_light_path)) {
                        if ((!infinite_world || unnatural_limit.intersect(intersection.geo.p))
#ifdef ISLAND_MODE
                            && frustum.intersect(intersection.geo.p, 0.1f)
#endif

                        ) {
                            auto& photon = photons[num_photons];

                            photon.p        = intersection.geo.p;
                            photon.wi       = wo;
                            photon.alpha[0] = radiance[0];
                            photon.alpha[1] = radiance[1];
                            photon.alpha[2] = radiance[2];
                            photon.properties.set(Photon::Property::First_hit, 0 == num_photons);
                            photon.properties.set(Photon::Property::Volumetric,
                                                  intersection.subsurface);

                            iteration = i + 1;

                            ++num_photons;

                            if (max_photons == num_photons || caustics_only) {
                                return iteration;
                            }
                        }
                    }

                    if (!settings_.indirect_caustics) {
                        break;
                    }
                }

                float3 const nr  = radiance * sample_result.reflection / sample_result.pdf;
                float const  avg = average(nr) / average(radiance);

                float const continue_prob = std::min(1.f, avg);

                if (sampler_.generate_sample_1D() > continue_prob) {
                    break;
                }

                radiance = nr / continue_prob;

                ray.set_direction(sample_result.wi);
                ++ray.depth;
            }

            ray.origin = material_sample.offset_p(intersection.geo.p, sample_result.wi);
            ray.min_t  = 0.f;
            ray.max_t  = scene::Ray_max_t;

            if (0.f == ray.wavelength) {
                ray.wavelength = sample_result.wavelength;
            }

            if (sample_result.type.is(Bxdf_type::Transmission)) {
                auto const ior = worker.interface_change_ior(sample_result.wi, intersection);

#ifdef ISLAND_MODE
                if (worker.interface_stack().empty()) {
                    break;
                }
#endif

                float const eta = ior.eta_i / ior.eta_t;

                radiance *= eta * eta;
            }

            ray.max_t = scene::Ray_max_t;

            if (!worker.interface_stack().empty()) {
                float3     vli, vtr;
                auto const hit = worker.volume(ray, intersection, filter, vli, vtr);

                //   radiance += throughput * vli;
                radiance *= vtr;

                if (Event::Abort == hit) {
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

bool Mapper::generate_light_ray(uint32_t frame, AABB const& bounds, Worker& worker, Ray& ray,
                                Light& light_out, uint32_t& light_id,
                                Sample_from& light_sample) noexcept {
    float const select = sampler_.generate_sample_1D(1);

    auto const light = worker.scene().random_light(select);

    uint64_t const time = worker.absolute_time(frame, sampler_.generate_sample_1D(2));

    Importance const& importance = worker.particle_importance().importance(light.id);

    if (importance.distribution().empty()) {
        if (!light.ref.sample(time, sampler_, 0, bounds, worker, light_sample)) {
            return false;
        }
    } else {
        if (!light.ref.sample(time, sampler_, 0, importance.distribution(), bounds, worker,
                              light_sample)) {
            return false;
        }

        light_sample.pdf *= importance.denormalization_factor();
    }

    ray.origin = scene::offset_ray(light_sample.p, light_sample.dir);
    ray.set_direction(light_sample.dir);
    ray.min_t      = 0.f;
    ray.max_t      = scene::Ray_max_t;
    ray.depth      = 0;
    ray.time       = time;
    ray.wavelength = 0.f;

    light_out = light.ref;
    light_id  = light.id;

    light_sample.pdf *= light.pdf;

    return true;
}

}  // namespace rendering::integrator::particle::photon
