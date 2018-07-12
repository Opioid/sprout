#include "prop_image_light.hpp"
#include "base/math/aabb.inl"
#include "base/math/math.hpp"
#include "base/math/matrix4x4.inl"
#include "base/math/vector3.inl"
#include "light_sample.hpp"
#include "sampler/sampler.hpp"
#include "scene/material/material.hpp"
#include "scene/prop/prop.hpp"
#include "scene/scene_ray.hpp"
#include "scene/scene_worker.hpp"
#include "scene/shape/shape.hpp"
#include "scene/shape/shape_intersection.hpp"
#include "scene/shape/shape_sample.hpp"

namespace scene::light {

bool Prop_image_light::sample(f_float3 p, f_float3 n, Transformation const& transformation,
                              bool total_sphere, sampler::Sampler& sampler,
                              uint32_t   sampler_dimension, Worker const& /*worker*/,
                              Sample_to& result) const {
    auto const material = prop_->material(part_);

    float2 const s2d = sampler.generate_sample_2D(sampler_dimension);

    auto const rs = material->radiance_sample(s2d);
    if (0.f == rs.pdf) {
        return false;
    }

    float const area = prop_->area(part_);

    bool const two_sided = material->is_two_sided();

    // this pdf includes the uv weight which adjusts for texture distortion by the shape
    if (!prop_->shape()->sample(part_, p, rs.uv, transformation, area, two_sided, result)) {
        return false;
    }

    if (math::dot(result.wi, n) > 0.f || total_sphere) {
        result.pdf *= rs.pdf;
        return true;
    }

    return false;
}

bool Prop_image_light::sample(f_float3 p, Transformation const& transformation,
                              sampler::Sampler& sampler, uint32_t  sampler_dimension,
                              Worker const& /*worker*/, Sample_to& result) const {
    auto const material = prop_->material(part_);

    float2 const s2d = sampler.generate_sample_2D(sampler_dimension);

    auto const rs = material->radiance_sample(s2d);
    if (0.f == rs.pdf) {
        return false;
    }

    float const area = prop_->area(part_);

    bool const two_sided = material->is_two_sided();

    // this pdf includes the uv weight which adjusts for texture distortion by the shape
    if (!prop_->shape()->sample(part_, p, rs.uv, transformation, area, two_sided, result)) {
        return false;
    }

    result.pdf *= rs.pdf;

    return true;
}

bool Prop_image_light::sample(float /*time*/, Transformation const& transformation,
                              sampler::Sampler& sampler, uint32_t sampler_dimension,
                              Sampler_filter /*filter*/, Worker const& /*worker*/,
                              Sample_from& result) const {
    auto const material = prop_->material(part_);

    float2 const s2d = sampler.generate_sample_2D(sampler_dimension);

    auto const rs = material->radiance_sample(s2d);
    if (0.f == rs.pdf) {
        return false;
    }

    float const area = prop_->area(part_);

    bool const two_sided = material->is_two_sided();

    // this pdf includes the uv weight which adjusts for texture distortion by the shape
    if (!prop_->shape()->sample(part_, rs.uv, transformation, area, two_sided, result.shape)) {
        return false;
    }

    result.shape.pdf *= rs.pdf;

    return true;
}

float Prop_image_light::pdf(Ray const& ray, Intersection const& intersection, bool /*total_sphere*/,
                            Sampler_filter filter, Worker const& worker) const {
    Transformation temp;
    auto const&    transformation = prop_->transformation_at(ray.time, temp);

    float const area = prop_->area(part_);

    auto const material = prop_->material(part_);

    bool const two_sided = material->is_two_sided();

    // this pdf includes the uv weight which adjusts for texture distortion by the shape
    float const shape_pdf = prop_->shape()->pdf_uv(ray, intersection, transformation, area,
                                                   two_sided);

    float const material_pdf = material->emission_pdf(intersection.uv, filter, worker);

    return shape_pdf * material_pdf;
}

void Prop_image_light::prepare_sampling(uint32_t light_id, thread::Pool& pool) {
    prop_->prepare_sampling(part_, light_id, true, pool);
}

}  // namespace scene::light
