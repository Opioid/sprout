#include "prop_light.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "sampler/sampler.hpp"
#include "scene/material/material.hpp"
#include "scene/prop/prop.hpp"
#include "scene/scene.hpp"
#include "scene/scene_ray.hpp"
#include "scene/scene_worker.hpp"
#include "scene/shape/shape.hpp"
#include "scene/shape/shape_sample.hpp"

namespace scene::light {

void Prop_light::init(uint32_t prop, uint32_t part) noexcept {
    prop_ = prop;
    part_ = part;
}

Light::Transformation const& Prop_light::transformation_at(uint64_t        time,
                                                           Transformation& transformation,
                                                           Scene const&    scene) const noexcept {
    return scene.prop(prop_)->transformation_at(prop_, time, transformation, scene);
}

bool Prop_light::sample(float3 const& p, float3 const& n, Transformation const& transformation,
                        bool total_sphere, Sampler& sampler, uint32_t sampler_dimension,
                        Worker const& worker, Sample_to& result) const noexcept {
    Prop const* prop = worker.scene().prop(prop_);

    auto const material = worker.scene().prop_material(prop_, part_);

    float const area = worker.scene().prop_area(prop_, part_);

    bool const two_sided = material->is_two_sided();

    if (total_sphere) {
        if (!prop->shape()->sample(part_, p, transformation, area, two_sided, sampler,
                                   sampler_dimension, worker.node_stack(), result)) {
            return false;
        }
    } else {
        if (!prop->shape()->sample(part_, p, n, transformation, area, two_sided, sampler,
                                   sampler_dimension, worker.node_stack(), result)) {
            return false;
        }

        if (dot(result.wi, n) <= 0.f) {
            return false;
        }
    }

    return true;
}

float3 Prop_light::evaluate(Sample_to const& sample, Filter filter, Worker const& worker) const
    noexcept {
    Prop const* prop = worker.scene().prop(prop_);

    auto const material = worker.scene().prop_material(prop_, part_);

    float const area = worker.scene().prop_area(prop_, part_);

    return material->evaluate_radiance(sample.wi, sample.uvw.xy(), area, filter, worker);
}

bool Prop_light::sample(Transformation const& transformation, Sampler& sampler,
                        uint32_t sampler_dimension, AABB const& bounds, Worker const& worker,
                        Sample_from& result) const noexcept {
    Prop const* prop = worker.scene().prop(prop_);

    auto const material = worker.scene().prop_material(prop_, part_);

    float const area = worker.scene().prop_area(prop_, part_);

    bool const two_sided = material->is_two_sided();

    float2 const importance_uv = sampler.generate_sample_2D();

    if (!prop->shape()->sample(part_, transformation, area, two_sided, sampler, sampler_dimension,
                               importance_uv, bounds, worker.node_stack(), result)) {
        return false;
    }

    return true;
}

bool Prop_light::sample(Transformation const& transformation, Sampler& sampler,
                        uint32_t sampler_dimension, Distribution_2D const& importance,
                        AABB const& bounds, Worker const& worker, Sample_from& result) const
    noexcept {
    Prop const* prop = worker.scene().prop(prop_);

    auto const material = worker.scene().prop_material(prop_, part_);

    float const area = worker.scene().prop_area(prop_, part_);

    bool const two_sided = material->is_two_sided();

    float2 const s2d = sampler.generate_sample_2D();

    auto const importance_uv = importance.sample_continuous(s2d);
    if (0.f == importance_uv.pdf) {
        return false;
    }

    if (!prop->shape()->sample(part_, transformation, area, two_sided, sampler, sampler_dimension,
                               importance_uv.uv, bounds, worker.node_stack(), result)) {
        return false;
    }

    result.pdf *= importance_uv.pdf;

    return true;
}

float3 Prop_light::evaluate(Sample_from const& sample, Filter filter, Worker const& worker) const
    noexcept {
    auto const material = worker.scene().prop_material(prop_, part_);

    float const area = worker.scene().prop_area(prop_, part_);

    return material->evaluate_radiance(-sample.dir, sample.uv, area, filter, worker);
}

float Prop_light::pdf(Ray const& ray, Intersection const& intersection, bool total_sphere,
                      Filter /*filter*/, Worker const& worker) const noexcept {
    Prop const* prop = worker.scene().prop(prop_);

    Transformation temp;
    auto const&    transformation = prop->transformation_at(prop_, ray.time, temp, worker.scene());

    float const area = worker.scene().prop_area(prop_, part_);

    bool const two_sided = worker.scene().prop_material(prop_, part_)->is_two_sided();

    return prop->shape()->pdf(ray, intersection, transformation, area, two_sided, total_sphere);
}

float3 Prop_light::power(AABB const& scene_bb, Scene const& scene) const noexcept {
    Prop const* prop = scene.prop(prop_);

    float const area = scene.prop_area(prop_, part_);

    float3 const radiance = scene.prop_material(prop_, part_)->average_radiance(area);

    if (prop->shape()->is_finite()) {
        return area * radiance;
    } else {
        return squared_length(scene_bb.halfsize()) * area * radiance;
    }
}

void Prop_light::prepare_sampling(uint32_t light_id, uint64_t time, Scene& scene,
                                  thread::Pool& pool) noexcept {
    //   scene.prop(prop_)->prepare_sampling(prop_, part_, light_id, time, false, pool, scene);

    scene.prop_prepare_sampling(prop_, part_, light_id, time, false, pool);
}

bool Prop_light::equals(uint32_t prop, uint32_t part) const noexcept {
    return prop_ == prop && part_ == part;
}

}  // namespace scene::light
