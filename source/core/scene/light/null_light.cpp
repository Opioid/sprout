#include "null_light.hpp"
#include "base/math/vector3.inl"

namespace scene::light {

Light::Transformation const& Null_light::transformation_at(uint64_t /*time*/,
                                                           Transformation& transformation) const
    noexcept {
    return transformation;
}

bool Null_light::sample(float3 const& /*p*/, float3 const& /*n*/,
                        Transformation const& /*transformation*/, bool /*total_sphere*/,
                        sampler::Sampler& /*sampler*/, uint32_t /*sampler_dimension*/,
                        Worker const& /*worker*/, Sample_to& /*result*/) const noexcept {
    return false;
}

bool Null_light::sample(float3 const& /*p*/, Transformation const& /*transformation*/,
                        sampler::Sampler& /*sampler*/, uint32_t /*sampler_dimension*/,
                        Worker const& /*worker*/, Sample_to& /*result*/) const noexcept {
    return false;
}

float3 Null_light::evaluate(Sample_to const& /*sample*/, uint64_t /*time*/, Sampler_filter /*filter*/,
                            Worker const& /*worker*/) const noexcept {
    return float3(0.f);
}

bool Null_light::sample(Transformation const& /*transformation*/, sampler::Sampler& /*sampler*/,
                        uint32_t /*sampler_dimension*/, math::AABB const& /*bounds*/,
                        Worker const& /*worker*/, Sample_from& /*result*/) const noexcept {
    return false;
}

float3 Null_light::evaluate(Sample_from const& /*sample*/, uint64_t /*time*/,
                            Sampler_filter /*filter*/, Worker const& /*worker*/) const noexcept {
    return float3(0.f);
}

float Null_light::pdf(Ray const& /*ray*/, Intersection const& /*intersection*/,
                      bool /*total_sphere*/, Sampler_filter /*filter*/,
                      Worker const& /*worker*/) const noexcept {
    return 0.f;
}

float3 Null_light::power(math::AABB const& /*scene_bb*/) const noexcept {
    return float3::identity();
}

void Null_light::prepare_sampling(uint32_t /*light_id*/, thread::Pool& /*pool*/) noexcept {}

bool Null_light::equals(Prop const* /*prop*/, uint32_t /*part*/) const noexcept {
    return false;
}

}  // namespace scene::light
