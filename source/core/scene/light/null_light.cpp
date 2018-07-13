#include "null_light.hpp"
#include "base/math/vector3.inl"

namespace scene::light {

const Light::Transformation& Null_light::transformation_at(float /*time*/,
                                                           Transformation& transformation) const {
    return transformation;
}

bool Null_light::sample(f_float3 /*p*/, f_float3 /*n*/, Transformation const& /*transformation*/,
                        bool /*total_sphere*/, sampler::Sampler& /*sampler*/,
                        uint32_t /*sampler_dimension*/, Worker const& /*worker*/,
                        Sample_to& /*result*/) const {
    return false;
}

bool Null_light::sample(f_float3 /*p*/, Transformation const& /*transformation*/,
                        sampler::Sampler& /*sampler*/, uint32_t /*sampler_dimension*/,
                        Worker const& /*worker*/, Sample_to& /*result*/) const {
    return false;
}

float3 Null_light::evaluate(Sample_to const& /*sample*/, float /*time*/, Sampler_filter /*filter*/,
                            Worker const& /*worker*/) const {
    return float3(0.f);
}

bool Null_light::sample(Transformation const& /*transformation*/,
                        sampler::Sampler& /*sampler*/, uint32_t /*sampler_dimension*/,
                        Worker const& /*worker*/,
                        Sample_from& /*result*/) const {
    return false;
}

float3 Null_light::evaluate(Sample_from const& /*sample*/, float /*time*/, Sampler_filter /*filter*/,
                            Worker const& /*worker*/) const {
    return float3(0.f);
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
