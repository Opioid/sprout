#include "prop_volume_light.hpp"
#include "base/math/vector3.inl"
#include "scene/prop/prop.hpp"
#include "scene/scene_worker.hpp"
#include "scene/shape/shape.hpp"
#include "scene/shape/shape_sample.hpp"

namespace scene::light {

bool Prop_volume_light::sample(float3 const& p, float3 const& n,
                               Transformation const& transformation, bool /*total_sphere*/,
                               Sampler& sampler, uint32_t sampler_dimension, Worker const& worker,
                               Sample_to& result) const noexcept {
    float const volume = prop_->volume(part_);

    if (!prop_->shape()->sample_volume(part_, p, transformation, volume, sampler, sampler_dimension,
                                       worker.node_stack(), result)) {
        return false;
    }

    if (dot(result.wi, n) <= 0.f) {
        return false;
    }

    return true;
}

float3 Prop_volume_light::evaluate(Sample_to const& sample, Filter filter,
                                   Worker const& worker) const noexcept {
    auto const material = prop_->material(part_);

    float const area = prop_->area(part_);

    return material->evaluate_radiance(sample.wi, sample.uv, area, filter, worker);
}

bool Prop_volume_light::sample(Transformation const& transformation, Sampler& sampler,
                               uint32_t sampler_dimension, AABB const& bounds, Worker const& worker,
                               Sample_from& result) const noexcept {
    return false;
}

float Prop_volume_light::pdf(Ray const& ray, Intersection const& intersection, bool total_sphere,
                             Filter filter, Worker const& worker) const noexcept {
    return 0.f;
}

float3 Prop_volume_light::power(AABB const& /*scene_bb*/) const noexcept {
    float const volume = prop_->volume(part_);

    float3 const radiance = prop_->material(part_)->average_radiance(volume);

    return volume * radiance;
}

void Prop_volume_light::prepare_sampling(uint32_t light_id, uint64_t time,
                                         thread::Pool& pool) noexcept {
    prop_->prepare_sampling_volume(part_, light_id, time, false, pool);
}

}  // namespace scene::light
