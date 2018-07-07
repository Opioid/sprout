#include "tracking.hpp"
#include "base/math/matrix4x4.inl"
#include "base/math/vector3.inl"
#include "base/random/generator.inl"
#include "rendering/integrator/integrator_helper.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/entity/composed_transformation.inl"
#include "scene/material/volumetric/volumetric_octree.hpp"
#include "scene/prop/interface_stack.inl"
#include "scene/prop/prop_intersection.inl"
#include "scene/scene_ray.inl"
#include "scene/shape/shape.hpp"

#include "base/debug/assert.hpp"

namespace rendering::integrator::volume {

// Code for hetereogeneous transmittance from:
// https://github.com/DaWelter/ToyTrace/blob/master/atmosphere.cxx

using Material       = scene::material::Material;
using Sampler_filter = scene::material::Sampler_settings::Filter;

static inline bool track_transmitted(float3& transmitted, math::Ray const& ray,
                                     float2 minorant_majorant, Material const& material,
                                     Sampler_filter filter, rnd::Generator& rng, Worker& worker) {
    static float constexpr Abort_epsilon = 1e-6f;

    float const mt = minorant_majorant[1];

    if (mt < Tracking::Min_mt) {
        return true;
    }

    if (minorant_majorant[0] == minorant_majorant[1]) {
        // Homogeneous segment
        transmitted *= attenuation(ray.max_t - ray.min_t, minorant_majorant[0]);

        return math::all_greater_equal(transmitted, Abort_epsilon);
    }

    float const imt = 1.f / mt;

    SOFT_ASSERT(std::isfinite(imt));

    float const d = ray.max_t;

    for (float t = ray.min_t;;) {
        float const r0 = rng.random_float();
        t -= std::log(1.f - r0) * imt;
        if (t > d) {
            return true;
        }

        float3 const uvw = ray.point(t);

        auto const mu = material.collision_coefficients(uvw, filter, worker);

        float3 const mu_t = mu.a + mu.s;

        float3 const mu_n = float3(mt) - mu_t;

        transmitted *= imt * mu_n;

        // TODO: employ russian roulette instead of just aborting
        if (math::all_lesser(transmitted, Abort_epsilon)) {
            return false;
        }

        SOFT_ASSERT(math::all_finite(w));
    }
}

float3 Tracking::transmittance(Ray const& ray, rnd::Generator& rng, Worker& worker) {
    SOFT_ASSERT(!worker.interface_stack().empty());

    auto const interface = worker.interface_stack().top();

    auto const& material = *interface->material();

    float const d = ray.max_t;

    if (d - ray.min_t < Ray_epsilon) {
        return float3(1.f);
    }

    if (material.is_heterogeneous_volume()) {
        Transformation temp;
        auto const&    transformation = interface->prop->transformation_at(ray.time, temp);

        float3 const local_origin = transformation.world_to_object_point(ray.origin);
        float3 const local_dir    = transformation.world_to_object_vector(ray.direction);

        auto const   shape  = interface->prop->shape();
        float3 const origin = shape->object_to_texture_point(local_origin);
        float3 const dir    = shape->object_to_texture_vector(local_dir);

        auto const& tree = *material.volume_tree();

        math::Ray local_ray(origin, dir, ray.min_t, ray.max_t);

        const float ray_offset = Ray_epsilon / math::length(dir);

        float3 w(1.f);
        for (; local_ray.min_t < d;) {
            if (float2 mi_ma; tree.intersect(local_ray, mi_ma)) {
                if (!track_transmitted(w, local_ray, mi_ma, material, Sampler_filter::Nearest, rng,
                                       worker)) {
                    return float3::identity();
                }
            }

            SOFT_ASSERT(local_ray.max_t + ray_offset > local_ray.min_t);

            local_ray.min_t = local_ray.max_t + ray_offset;
            local_ray.max_t = d;
        }

        return w;
    } else {
        auto const mu = material.collision_coefficients(interface->uv, Sampler_filter::Nearest,
                                                        worker);

        float3 const mu_t = mu.a + mu.s;

        return attenuation(d - ray.min_t, mu_t);
    }
}

bool Tracking::track(math::Ray const& ray, float2 minorant_majorant, Material const& material,
                     Sampler_filter filter, rnd::Generator& rng, Worker& worker, float& t_out,
                     float3& w) {
    float const mt = minorant_majorant[1];

    if (mt < Min_mt) {
        return false;
    }

    float3 lw = w;

    SOFT_ASSERT(math::all_finite(lw));

    float const imt = 1.f / mt;

    float const d = ray.max_t;

    for (float t = ray.min_t;;) {
        float const r0 = rng.random_float();
        t -= std::log(1.f - r0) * imt;
        if (t > d) {
            w = lw;
            return false;
        }

        float3 const uvw = ray.point(t);

        auto const mu = material.collision_coefficients(uvw, filter, worker);

        float3 const mu_t = mu.a + mu.s;

        float3 const mu_n = float3(mt) - mu_t;

        float const ms = math::average(mu.s * lw);
        float const mn = math::average(mu_n * lw);
        float const c  = 1.f / (ms + mn);

        float const ps = ms * c;
        float const pn = mn * c;

        float const r1 = rng.random_float();
        if (r1 <= 1.f - pn && ps > 0.f) {
            float3 const ws = mu.s / (mt * ps);

            SOFT_ASSERT(math::all_finite(ws));

            t_out = t;
            w     = lw * ws;
            return true;
        } else {
            float3 const wn = mu_n / (mt * pn);

            SOFT_ASSERT(math::all_finite(wn));

            lw *= wn;
        }
    }
}

}  // namespace rendering::integrator::volume
