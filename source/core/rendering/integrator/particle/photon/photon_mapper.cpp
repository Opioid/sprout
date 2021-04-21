#include "photon_mapper.hpp"
#include "base/math/aabb.inl"
#include "base/math/frustum.hpp"
#include "photon.hpp"
#include "photon_map.hpp"
#include "rendering/integrator/integrator_helper.hpp"
#include "rendering/integrator/particle/particle_importance.hpp"
#include "rendering/rendering_worker.inl"
#include "scene/camera/camera.hpp"
#include "scene/light/light.inl"
#include "scene/material/bxdf.hpp"
#include "scene/material/material.inl"
#include "scene/material/material_sample.inl"
#include "scene/prop/interface_stack.inl"
#include "scene/prop/prop_intersection.inl"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/shape/shape_sample.hpp"

//#define ISLAND_MODE

namespace rendering::integrator::particle::photon {

Mapper::Mapper(Settings const& settings)
    : settings_(settings), photons_(new Photon[settings.max_bounces]) {}

Mapper::~Mapper() {
    delete[] photons_;
}

void Mapper::start_pixel(rnd::Generator& /*rng*/) {}

uint32_t Mapper::bake(Map& map, int32_t begin, int32_t end, uint32_t frame, uint32_t /*iteration*/,
                      Worker& worker) {
    Frustum const frustum = worker.camera().frustum();

    AABB const world_bounds = settings_.full_light_path ? worker.scene().aabb()
                                                        : worker.scene().caustic_aabb();

    AABB const bounds = world_bounds.intersection(frustum.calculate_aabb());

    bool const infinite_world = worker.scene().is_infinite();
    bool const caustics_only  = map.caustics_only();

    uint32_t num_paths = 0;

    for (int32_t i = begin; i < end;) {
        uint32_t const max_photons = std::min(settings_.max_bounces, uint32_t(end - i));

        uint32_t       num_photons;
        uint32_t const num_iterations = trace_photon(frame, bounds, frustum, infinite_world,
                                                     caustics_only, worker, max_photons, photons_,
                                                     num_photons);

        if (num_iterations > 0) {
            for (uint32_t j = 0; j < num_photons; ++j) {
                Photon const& ph = photons_[j];

                map.insert(ph, uint32_t(i) + j);
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
                              uint32_t max_photons, Photon* photons, uint32_t& num_photons) {
    // How often should we try to create a valid photon path?
    static uint32_t constexpr Max_iterations = 1024 * 10;

    AABB unnatural_limit = bounds;
    unnatural_limit.scale(8.f);

    bool constexpr avoid_caustics = false;

    uint32_t iteration = 0;

    num_photons = 0;

    for (uint32_t i = 0; i < Max_iterations; ++i) {
        worker.interface_stack().clear();

        Bxdf_sample sample_result;

        Intersection isec;

        Filter filter = Filter::Undefined;

        bool caustic_path    = false;
        bool from_subsurface = false;

        Ray         ray;
        uint32_t    light_id;
        Sample_from light_sample;
        if (!generate_light_ray(frame, bounds, worker, ray, light_id, light_sample)) {
            continue;
        }

        if (!worker.intersect_and_resolve_mask(ray, isec, filter)) {
            continue;
        }

        auto const& light = worker.scene().light(light_id);

        float3 radiance = light.evaluate(light_sample, Filter::Nearest, worker) /
                          (light_sample.pdf);

        float3 wo1(0.f);

        for (; ray.depth < settings_.max_bounces;) {
            float3 const wo = -ray.direction;

            auto const& mat_sample = worker.sample_material(
                ray, wo, wo1, isec, filter, 0.f, avoid_caustics, from_subsurface, sampler_);

            wo1 = wo;

            if (mat_sample.is_pure_emissive()) {
                break;
            }

            mat_sample.sample(sampler_, worker.rng(), sample_result);
            if (0.f == sample_result.pdf) {
                break;
            }

#ifdef ISLAND_MODE
            if (0 == ray.depth && sample_result.type.no(Bxdf_type::Transmission)) {
                break;
            }
#endif

            if (sample_result.type.no(Bxdf_type::Straight)) {
                if (sample_result.type.no(Bxdf_type::Specular) &&
                    (isec.subsurface | mat_sample.same_hemisphere(wo)) &&
                    (caustic_path | settings_.full_light_path)) {
                    if ((!infinite_world || unnatural_limit.intersect(isec.geo.p))
#ifdef ISLAND_MODE
                        && frustum.intersect(isec.geo.p, 0.1f)
#endif

                    ) {
                        auto& photon = photons[num_photons];

                        float3 radi = radiance;

                        if (isec.subsurface && (isec.material(worker)->ior() > 1.f)) {
                            float const ior_t = worker.interface_stack().next_to_bottom_ior(worker);
                            float const eta   = isec.material(worker)->ior() / ior_t;

                            radi *= eta * eta;
                        }

                        photon.p        = isec.geo.p;
                        photon.wi       = wo;
                        photon.alpha[0] = radi[0];
                        photon.alpha[1] = radi[1];
                        photon.alpha[2] = radi[2];
                        photon.properties.set(Photon::Property::Volumetric, isec.subsurface);

                        iteration = i + 1;

                        ++num_photons;

                        float const w = std::max(radi[0], std::max(radi[1], radi[2]));
                        worker.particle_importance().increment(light_id, light_sample.xy, isec,
                                                               ray.time, w, worker);

                        if (max_photons == num_photons || caustics_only) {
                            return iteration;
                        }
                    }
                }

                if (sample_result.type.is(Bxdf_type::Specular)) {
                    caustic_path = true;
                } else {
                    filter = Filter::Nearest;
                }

                float3 const nr  = radiance * sample_result.reflection / sample_result.pdf;
                float const  avg = average(nr) / average(radiance);

                float const continue_prob = std::min(1.f, avg);

                if (sampler_.sample_1D(worker.rng()) > continue_prob) {
                    break;
                }

                radiance = nr / continue_prob;
            }

            if (sample_result.type.is(Bxdf_type::Straight)) {
                ray.min_t() = scene::offset_f(ray.max_t());

                if (sample_result.type.no(Bxdf_type::Transmission)) {
                    ++ray.depth;
                }
            } else {
                ray.origin = isec.offset_p(sample_result.wi);
                ray.set_direction(sample_result.wi);
                ++ray.depth;

                from_subsurface = false;
            }

            ray.max_t() = scene::Ray_max_t;

            if (0.f == ray.wavelength) {
                ray.wavelength = sample_result.wavelength;
            }

            if (sample_result.type.is(Bxdf_type::Transmission)) {
                auto const ior = worker.interface_change_ior(sample_result.wi, isec);

#ifdef ISLAND_MODE
                if (worker.interface_stack().empty()) {
                    break;
                }
#endif
                float const eta = ior.eta_i / ior.eta_t;

                radiance *= eta * eta;
            }

            from_subsurface |= isec.subsurface;

            if (!worker.interface_stack().empty()) {
                float3     vli;
                float3     vtr;
                auto const hit = worker.volume(ray, isec, filter, vli, vtr);

                //   radiance += throughput * vli;
                radiance *= vtr;

                if (Event::Abort == hit) {
                    break;
                }
            } else if (!worker.intersect_and_resolve_mask(ray, isec, filter)) {
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
                                uint32_t& light_id, Sample_from& light_sample) {
    auto& rng = worker.rng();

    float const select = sampler_.sample_1D(rng, 1);

    auto const light = worker.scene().random_light(select);

    uint64_t const time = worker.absolute_time(frame, sampler_.sample_1D(rng, 2));

    Importance const& importance = worker.particle_importance().importance(light.offset);

    if (importance.valid()) {
        if (!light.ref.sample(time, sampler_, 0, importance.distribution(), bounds, worker,
                              light_sample)) {
            return false;
        }

        light_sample.pdf *= importance.denormalization_factor();
    } else {
        if (!light.ref.sample(time, sampler_, 0, bounds, worker, light_sample)) {
            return false;
        }
    }

    ray.origin = light_sample.p;
    ray.set_direction(light_sample.dir);
    ray.min_t()    = 0.f;
    ray.max_t()    = scene::Ray_max_t;
    ray.depth      = 0;
    ray.time       = time;
    ray.wavelength = 0.f;

    light_id = light.offset;

    light_sample.pdf *= light.pdf;

    return true;
}

}  // namespace rendering::integrator::particle::photon
