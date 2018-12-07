#include "prop_light.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "scene/material/material.hpp"
#include "scene/prop/prop.hpp"
#include "scene/scene_ray.hpp"
#include "scene/scene_worker.hpp"
#include "scene/shape/shape.hpp"
#include "scene/shape/shape_sample.hpp"

namespace scene::light {

void Prop_light::init(Prop* prop, uint32_t part) noexcept {
    prop_ = prop;
    part_ = part;
}

Light::Transformation const& Prop_light::transformation_at(uint64_t        time,
                                                           Transformation& transformation) const
    noexcept {
    return prop_->transformation_at(time, transformation);
}

bool Prop_light::sample(float3 const& p, float3 const& n, Transformation const& transformation,
                        bool total_sphere, Sampler& sampler, uint32_t sampler_dimension,
                        Worker const& worker, Sample_to& result) const noexcept {
    auto const material = prop_->material(part_);

    float const area = prop_->area(part_);

    bool const two_sided = material->is_two_sided();

    if (total_sphere) {
        if (!prop_->shape()->sample(part_, p, transformation, area, two_sided, sampler,
                                    sampler_dimension, worker.node_stack(), result)) {
            return false;
        }
    } else {
        if (!prop_->shape()->sample(part_, p, n, transformation, area, two_sided, sampler,
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
    auto const material = prop_->material(part_);

    float const area = prop_->area(part_);

    return material->evaluate_radiance(sample.wi, sample.uvw.xy(), area, filter, worker);
}

bool Prop_light::sample(Transformation const& transformation, Sampler& sampler,
                        uint32_t sampler_dimension, AABB const& bounds, Worker const& worker,
                        Sample_from& result) const noexcept {
    auto const material = prop_->material(part_);

    float const area = prop_->area(part_);

    bool const two_sided = material->is_two_sided();

    if (!prop_->shape()->sample(part_, transformation, area, two_sided, sampler, sampler_dimension,
                                bounds, worker.node_stack(), result)) {
        return false;
    }

    return true;
}

float3 Prop_light::evaluate(Sample_from const& sample, Filter filter, Worker const& worker) const
    noexcept {
    auto const material = prop_->material(part_);

    float const area = prop_->area(part_);

    return material->evaluate_radiance(-sample.dir, sample.uv, area, filter, worker);
}

float Prop_light::pdf(Ray const& ray, Intersection const& intersection, bool total_sphere,
                      Filter /*filter*/, Worker const& /*worker*/) const noexcept {
    Transformation temp;
    auto const&    transformation = prop_->transformation_at(ray.time, temp);

    float const area = prop_->area(part_);

    bool const two_sided = prop_->material(part_)->is_two_sided();

    return prop_->shape()->pdf(ray, intersection, transformation, area, two_sided, total_sphere);
}

float3 Prop_light::power(AABB const& scene_bb) const noexcept {
    float const area = prop_->area(part_);

    float3 const radiance = prop_->material(part_)->average_radiance(area);

    if (prop_->shape()->is_finite()) {
        return area * radiance;
    } else {
        return squared_length(scene_bb.halfsize()) * area * radiance;
    }
}

void Prop_light::prepare_sampling(uint32_t light_id, uint64_t time, thread::Pool& pool) noexcept {
    prop_->prepare_sampling(part_, light_id, time, false, pool);
}

bool Prop_light::equals(Prop const* prop, uint32_t part) const noexcept {
    return prop_ == prop && part_ == part;
}

}  // namespace scene::light
