#include "prop_volume_image_light.hpp"
#include "base/math/vector3.inl"
#include "sampler/sampler.hpp"
#include "scene/prop/prop.hpp"
#include "scene/scene_ray.hpp"
#include "scene/scene_worker.hpp"
#include "scene/shape/shape.hpp"
#include "scene/shape/shape_sample.hpp"

namespace scene::light {

bool Prop_volume_image_light::sample(float3 const& p, float3 const& n,
                                     Transformation const& transformation, bool /*total_sphere*/,
                                     Sampler& sampler, uint32_t           sampler_dimension,
                                     Worker const& /*worker*/, Sample_to& result) const noexcept {
    auto const material = prop_->material(part_);

    float2 const s2d = sampler.generate_sample_2D(sampler_dimension);
    float const  s1d = sampler.generate_sample_1D(sampler_dimension);

    auto const rs = material->radiance_sample(float3(s2d, s1d));
    if (0.f == rs.pdf) {
        return false;
    }

    float const volume = prop_->volume(part_);

    // this pdf includes the uv weight which adjusts for texture distortion by the shape
    if (!prop_->shape()->sample(part_, p, rs.uvw, transformation, volume, result)) {
        return false;
    }

    if (dot(result.wi, n) > 0.f) {
        result.pdf *= rs.pdf;
        return true;
    }

    return false;
}

float Prop_volume_image_light::pdf(Ray const& ray, Intersection const& intersection,
                                   bool /*total_sphere*/, Filter /*filter*/,
                                   Worker const& /*worker*/) const noexcept {
    Transformation temp;
    auto const&    transformation = prop_->transformation_at(ray.time, temp);

    float const volume = prop_->volume(part_);

    return prop_->shape()->pdf_volume(ray, intersection, transformation, volume);
}

void Prop_volume_image_light::prepare_sampling(uint32_t light_id, uint64_t time,
                                               thread::Pool& pool) noexcept {
    prop_->prepare_sampling_volume(part_, Volume_light_mask | light_id, time, true, pool);
}

}  // namespace scene::light
