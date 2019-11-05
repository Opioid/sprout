#include "pathtracer_dldl.hpp"
#include "base/math/sampling.inl"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "base/spectrum/rgb.hpp"
#include "rendering/integrator/integrator_helper.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/light/light.inl"
#include "scene/material/bxdf.hpp"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.inl"
#include "scene/prop/interface_stack.inl"
#include "scene/prop/prop_intersection.inl"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/shape/shape_sample.hpp"

namespace rendering::integrator::surface {

using namespace scene;

Pathtracer_DLDL::Pathtracer_DLDL(rnd::Generator& rng, take::Settings const& take_settings,
                                 Settings const& settings) noexcept
    : Integrator(rng, take_settings), settings_(settings), sampler_(rng) {}

void Pathtracer_DLDL::prepare(Scene const& /*scene*/, uint32_t num_samples_per_pixel) noexcept {
    sampler_.resize(num_samples_per_pixel, 1, 1, 1);
}

void Pathtracer_DLDL::start_pixel() noexcept {
    sampler_.start_pixel();
}

float4 Pathtracer_DLDL::li(Ray& ray, Intersection& intersection, Worker& worker,
                           Interface_stack const& initial_stack) noexcept {
    //  float3 const wi = ray.direction;

    float3 const wi = normalize(float3(0.0001f, 0.9998f, 0.0001f));

    //    float2 const uv(rng_.random_float(), rng_.random_float());

    //    float3 const wi = sample_sphere_uniform(uv);

    float4 li(0.f);

    for (uint32_t i = settings_.num_samples; i > 0; --i) {
        worker.reset_interface_stack(initial_stack);

        Ray split_ray = ray;

        Intersection split_intersection = intersection;

        float4 result = integrate(split_ray, split_intersection, wi, worker);

        li += result;
    }

    float const num_samples_reciprocal = 1.f / float(settings_.num_samples);

    return num_samples_reciprocal * li;
}

float4 Pathtracer_DLDL::integrate(Ray& ray, Intersection& intersection, float3 const& wi,
                                  Worker& worker) noexcept {
    Filter filter = Filter::Undefined;

    Bxdf_sample sample_result;

    bool primary_ray       = true;
    bool treat_as_singular = true;
    bool evaluate_back     = true;
    bool transparent       = true;

    float3 throughput(1.f);
    float3 result(0.f);

    for (;;) {
        float3 const wo = -ray.direction;

        bool const avoid_caustics = false;

        auto& material_sample = intersection.sample(wo, ray, filter, avoid_caustics, sampler_,
                                                    worker);

        bool const same_side = material_sample.same_hemisphere(wo);

        if (treat_as_singular && same_side) {
            result += throughput * material_sample.radiance();
        }

        if (material_sample.is_pure_emissive()) {
            transparent &= !worker.scene().prop(intersection.prop)->visible_in_camera() &&
                           ray.max_t >= scene::Ray_max_t;
            break;
        }

        evaluate_back = material_sample.do_evaluate_back(evaluate_back, same_side);

        result += throughput * direct_light(ray, intersection, wi, material_sample, evaluate_back,
                                            filter, worker);

        SOFT_ASSERT(all_finite_and_positive(result));

        if (ray.depth >= settings_.max_bounces - 1) {
            break;
        }

        if (ray.depth > settings_.min_bounces) {
            if (russian_roulette(throughput, sampler_.generate_sample_1D())) {
                break;
            }
        }

        material_sample.sample(material_sampler(ray.depth), sample_result);
        if (0.f == sample_result.pdf) {
            break;
        }

        if (sample_result.type.is(Bxdf_type::Caustic)) {
            if (avoid_caustics) {
                break;
            }

            treat_as_singular = sample_result.type.is(Bxdf_type::Specular);
        } else if (sample_result.type.no(Bxdf_type::Straight)) {
            primary_ray       = false;
            filter            = Filter::Nearest;
            treat_as_singular = false;
        }

        if (0.f == ray.wavelength) {
            ray.wavelength = sample_result.wavelength;
        }

        if (material_sample.ior_greater_one()) {
            transparent &= sample_result.type.is(Bxdf_type::Straight);

            throughput *= sample_result.reflection / sample_result.pdf;

            ray.origin = material_sample.offset_p(intersection.geo.p, sample_result.wi);
            ray.min_t  = 0.f;
            ray.set_direction(sample_result.wi);
            ++ray.depth;
        } else {
            ray.min_t = scene::offset_f(ray.max_t);
        }

        ray.max_t = scene::Ray_max_t;

        if (sample_result.type.is(Bxdf_type::Transmission)) {
            worker.interface_change(sample_result.wi, intersection);
        }

        if (!worker.interface_stack().empty()) {
            float3     vli, vtr;
            auto const hit = worker.volume(ray, intersection, filter, vli, vtr);

            if (treat_as_singular) {
                result += throughput * vli;
            }

            throughput *= vtr;

            if (Event::Abort == hit || Event::Absorb == hit) {
                break;
            }
        } else if (!worker.intersect_and_resolve_mask(ray, intersection, filter)) {
            break;
        }
    }

    return compose_alpha(result, throughput, transparent);
}

float3 Pathtracer_DLDL::direct_light(Ray const& ray, Intersection const& intersection,
                                     float3 const& wi, Material_sample const& material_sample,
                                     bool evaluate_back, Filter filter, Worker& worker) noexcept {
    if ((0 == ray.depth) | !material_sample.ior_greater_one()) {
        return float3(0.f);
    }

    if (!material_sample.is_translucent() & (dot(material_sample.geometric_normal(), wi) <= 0.f)) {
        return float3(0.f);
    }

    float3 const p = material_sample.offset_p(intersection.geo.p);

    Ray shadow_ray;
    shadow_ray.origin     = p;
    shadow_ray.min_t      = 0.f;
    shadow_ray.depth      = ray.depth;
    shadow_ray.time       = ray.time;
    shadow_ray.wavelength = ray.wavelength;

    shadow_ray.set_direction(wi);
    shadow_ray.max_t = scene::Almost_ray_max_t;

    float3 tv;
    if (!worker.transmitted_visibility(shadow_ray, material_sample.wo(), intersection, filter,
                                       tv)) {
        return float3(0.f);
    }

    auto const bxdf = material_sample.evaluate_f(wi, evaluate_back);

    return tv * bxdf.reflection;
}

sampler::Sampler& Pathtracer_DLDL::material_sampler(uint32_t /*bounce*/) noexcept {
    return sampler_;
}

sampler::Sampler& Pathtracer_DLDL::light_sampler(uint32_t /*bounce*/) noexcept {
    return sampler_;
}

Pathtracer_DLDL_factory::Pathtracer_DLDL_factory(take::Settings const& take_settings,
                                                 uint32_t num_integrators, uint32_t num_samples,
                                                 uint32_t min_bounces,
                                                 uint32_t max_bounces) noexcept
    : Factory(take_settings),
      integrators_(memory::allocate_aligned<Pathtracer_DLDL>(num_integrators)) {
    settings_.num_samples = num_samples;
    settings_.min_bounces = min_bounces;
    settings_.max_bounces = max_bounces;
}

Pathtracer_DLDL_factory::~Pathtracer_DLDL_factory() noexcept {
    memory::free_aligned(integrators_);
}

Integrator* Pathtracer_DLDL_factory::create(uint32_t id, rnd::Generator& rng) const noexcept {
    return new (&integrators_[id]) Pathtracer_DLDL(rng, take_settings_, settings_);
}

}  // namespace rendering::integrator::surface
