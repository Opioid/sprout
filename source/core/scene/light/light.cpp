#include "light.hpp"
#include "base/math/matrix4x4.inl"
#include "base/math/vector3.inl"
#include "scene/entity/composed_transformation.hpp"

namespace scene::light {

Light::~Light() noexcept {}

bool Light::sample(float3 const& p, float3 const& n, uint64_t time, bool total_sphere,
                   Sampler& sampler, uint32_t sampler_dimension, Worker const& worker,
                   Sample_to& result) const noexcept {
    Transformation temp;
    auto const&    transformation = transformation_at(time, temp);

    return sample(p, n, transformation, total_sphere, sampler, sampler_dimension, worker, result);
}

bool Light::sample(float3 const& p, uint64_t time, Sampler& sampler, uint32_t sampler_dimension,
                   Worker const& worker, Sample_to& result) const noexcept {
    Transformation temp;
    auto const&    transformation = transformation_at(time, temp);

    return sample(p, float3(0.f), transformation, true, sampler, sampler_dimension, worker, result);
}

bool Light::sample(uint64_t time, Sampler& sampler, uint32_t sampler_dimension, AABB const& bounds,
                   Worker const& worker, Sample_from& result) const noexcept {
    Transformation temp;
    auto const&    transformation = transformation_at(time, temp);

    return sample(transformation, sampler, sampler_dimension, bounds, worker, result);
}

bool Light::is_light(uint32_t id) noexcept {
    return 0xFFFFFFFF != id;
}

bool Light::is_area_light(uint32_t id) noexcept {
    return 0 == (id & Volume_light_mask);
}

uint32_t Light::strip_mask(uint32_t id) noexcept {
    return ~Volume_light_mask & id;
}

}  // namespace scene::light
