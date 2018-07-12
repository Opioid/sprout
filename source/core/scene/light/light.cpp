#include "light.hpp"
#include "base/math/matrix4x4.inl"
#include "base/math/vector3.inl"
#include "scene/entity/composed_transformation.hpp"

namespace scene::light {

Light::~Light() {}

bool Light::sample(f_float3 p, f_float3 n, float time, bool total_sphere, sampler::Sampler& sampler,
                   uint32_t sampler_dimension, Worker const& worker, Sample_to& result) const {
    Transformation temp;
    auto const&    transformation = transformation_at(time, temp);

    return sample(p, n, transformation, total_sphere, sampler, sampler_dimension, worker, result);
}

bool Light::sample(f_float3 p, float time, sampler::Sampler& sampler, uint32_t sampler_dimension,
                   Worker const& worker, Sample_to& result) const {
    Transformation temp;
    auto const&    transformation = transformation_at(time, temp);

    return sample(p, transformation, sampler, sampler_dimension, worker, result);
}

bool Light::sample(float time, sampler::Sampler& sampler, uint32_t sampler_dimension,
                   Sampler_filter filter, Worker const& worker, Sample_from& result) const {
    Transformation temp;
    auto const&    transformation = transformation_at(time, temp);

    return sample(time, transformation, sampler, sampler_dimension, filter, worker, result);
}

bool Light::is_light(uint32_t id) {
    return 0xFFFFFFFF != id;
}

}  // namespace scene::light
