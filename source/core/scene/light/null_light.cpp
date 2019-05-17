#include "null_light.hpp"
#include "base/math/vector3.inl"

namespace scene::light {

Light::Transformation const& Null_light::transformation_at(uint64_t /*time*/,
                                                           Transformation& transformation,
                                                           Scene const& /*scene*/) const noexcept {
    return transformation;
}

bool Null_light::sample(float3 const& /*p*/, float3 const& /*n*/,
                        Transformation const& /*transformation*/, bool /*total_sphere*/,
                        Sampler& /*sampler*/, uint32_t /*sampler_dimension*/,
                        Worker const& /*worker*/, Sample_to& /*result*/) const noexcept {
    return false;
}

float3 Null_light::evaluate(Sample_to const& /*sample*/, Filter /*filter*/,
                            Worker const& /*worker*/) const noexcept {
    return float3(0.f);
}

bool Null_light::sample(Transformation const& /*transformation*/, Sampler& /*sampler*/,
                        uint32_t /*sampler_dimension*/, AABB const& /*bounds*/,
                        Worker const& /*worker*/, Sample_from& /*result*/) const noexcept {
    return false;
}

bool Null_light::sample(Transformation const& /*transformation*/, Sampler& /*sampler*/,
                        uint32_t /*sampler_dimension*/, Distribution_2D const& /*importance*/,
                        AABB const& /*bounds*/, Worker const& /*worker*/,
                        Sample_from& /*result*/) const noexcept {
    return false;
}
float3 Null_light::evaluate(Sample_from const& /*sample*/, Filter /*filter*/,
                            Worker const& /*worker*/) const noexcept {
    return float3(0.f);
}

float Null_light::pdf(Ray const& /*ray*/, Intersection const& /*intersection*/,
                      bool /*total_sphere*/, Filter /*filter*/, Worker const& /*worker*/) const
    noexcept {
    return 0.f;
}

float3 Null_light::power(AABB const& /*scene_bb*/, Scene const& /*scene*/) const noexcept {
    return float3(0.f);
}

void Null_light::prepare_sampling(uint32_t /*light_id*/, uint64_t /*time*/, Scene& /*scene*/,
                                  thread::Pool& /*pool*/) noexcept {}

bool Null_light::equals(uint32_t /*prop*/, uint32_t /*part*/) const noexcept {
    return false;
}

}  // namespace scene::light
