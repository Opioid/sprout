#include "null_light.hpp"
#include "base/math/vector3.inl"
#include "light_sample.hpp"

namespace scene::light {

const Light::Transformation& Null_light::transformation_at(float /*time*/,
                                                           Transformation& transformation) const {
    return transformation;
}

bool Null_light::sample(f_float3 /*p*/, f_float3 /*n*/, float /*time*/,
                        Transformation const& /*transformation*/, bool /*total_sphere*/,
                        sampler::Sampler& /*sampler*/, uint32_t /*sampler_dimension*/,
                        Sampler_filter /*filter*/, Worker const& /*worker*/,
                        Sample& /*result*/) const {
    return false;
}

bool Null_light::sample(f_float3 /*p*/, float /*time*/, Transformation const& /*transformation*/,
                        sampler::Sampler& /*sampler*/, uint32_t /*sampler_dimension*/,
                        Sampler_filter /*filter*/, Worker const& /*worker*/,
                        Sample& /*result*/) const {
    return false;
}

bool Null_light::sample(float /*time*/, Transformation const& /*transformation*/,
                        sampler::Sampler& /*sampler*/, uint32_t /*sampler_dimension*/,
                        Sampler_filter /*filter*/, Worker const& /*worker*/,
                        Sample& /*result*/) const {
    return false;
}

float Null_light::pdf(Ray const& /*ray*/, Intersection const& /*intersection*/,
                      bool /*total_sphere*/, Sampler_filter /*filter*/,
                      Worker const& /*worker*/) const {
    return 0.f;
}

float3 Null_light::power(math::AABB const& /*scene_bb*/) const {
    return float3::identity();
}

void Null_light::prepare_sampling(uint32_t /*light_id*/, thread::Pool& /*pool*/) {}

bool Null_light::equals(Prop const* /*prop*/, uint32_t /*part*/) const {
    return false;
}

}  // namespace scene::light
