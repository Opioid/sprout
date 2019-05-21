#include "prop_image_light.hpp"
#include "base/math/matrix4x4.inl"
#include "base/math/vector3.inl"
#include "sampler/sampler.hpp"
#include "scene/material/material.hpp"
#include "scene/prop/prop.hpp"
#include "scene/scene.hpp"
#include "scene/scene_ray.hpp"
#include "scene/scene_worker.hpp"
#include "scene/shape/shape.hpp"
#include "scene/shape/shape_intersection.hpp"
#include "scene/shape/shape_sample.hpp"

namespace scene::light {

bool Prop_image_light::sample(float3 const& p, float3 const& n,
                              Transformation const& transformation, bool total_sphere,
                              Sampler& sampler, uint32_t sampler_dimension, Worker const& worker,
                              Sample_to& result) const noexcept {
    Prop const* prop = worker.scene().prop(prop_);

    auto const material = worker.scene().prop_material(prop_, part_);

    float2 const s2d = sampler.generate_sample_2D(sampler_dimension);

    auto const rs = material->radiance_sample(s2d);
    if (0.f == rs.pdf) {
        return false;
    }

    float const area = worker.scene().prop_area(prop_, part_);

    bool const two_sided = material->is_two_sided();

    // this pdf includes the uv weight which adjusts for texture distortion by the shape
    if (!prop->shape()->sample(part_, p, rs.uv, transformation, area, two_sided, result)) {
        return false;
    }

    if (dot(result.wi, n) > 0.f || total_sphere) {
        result.pdf *= rs.pdf;
        return true;
    }

    return false;
}

bool Prop_image_light::sample(Transformation const& transformation, Sampler& sampler,
                              uint32_t sampler_dimension, AABB const& bounds, Worker const& worker,
                              Sample_from& result) const noexcept {
    Prop const* prop = worker.scene().prop(prop_);

    auto const material = worker.scene().prop_material(prop_, part_);

    float2 const s2d = sampler.generate_sample_2D(sampler_dimension);

    auto const rs = material->radiance_sample(s2d);
    if (0.f == rs.pdf) {
        return false;
    }

    float const area = worker.scene().prop_area(prop_, part_);

    bool const two_sided = material->is_two_sided();

    float2 const importance_uv = sampler.generate_sample_2D();

    // this pdf includes the uv weight which adjusts for texture distortion by the shape
    if (!prop->shape()->sample(part_, rs.uv, transformation, area, two_sided, sampler,
                               sampler_dimension, importance_uv, bounds, result)) {
        return false;
    }

    result.pdf *= rs.pdf;

    return true;
}

bool Prop_image_light::sample(Transformation const& transformation, Sampler& sampler,
                              uint32_t sampler_dimension, Distribution_2D const& importance,
                              AABB const& bounds, Worker const& worker, Sample_from& result) const
    noexcept {
    Prop const* prop = worker.scene().prop(prop_);

    auto const material = worker.scene().prop_material(prop_, part_);

    float2 const s2d0 = sampler.generate_sample_2D(sampler_dimension);

    auto const rs = material->radiance_sample(s2d0);
    if (0.f == rs.pdf) {
        return false;
    }

    float const area = worker.scene().prop_area(prop_, part_);

    bool const two_sided = material->is_two_sided();

    float2 const s2d1 = sampler.generate_sample_2D();

    auto const importance_uv = importance.sample_continuous(s2d1);
    if (0.f == importance_uv.pdf) {
        return false;
    }

    // this pdf includes the uv weight which adjusts for texture distortion by the shape
    if (!prop->shape()->sample(part_, rs.uv, transformation, area, two_sided, sampler,
                               sampler_dimension, importance_uv.uv, bounds, result)) {
        return false;
    }

    result.pdf *= rs.pdf * importance_uv.pdf;

    return true;
}

float Prop_image_light::pdf(Ray const& ray, Intersection const& intersection, bool /*total_sphere*/,
                            Filter filter, Worker const& worker) const noexcept {
    Prop const* prop = worker.scene().prop(prop_);

    Transformation temp;
    auto const&    transformation = prop->transformation_at(prop_, ray.time, temp, worker.scene());

    float const area = worker.scene().prop_area(prop_, part_);

    auto const material = worker.scene().prop_material(prop_, part_);

    bool const two_sided = material->is_two_sided();

    // this pdf includes the uv weight which adjusts for texture distortion by the shape
    float const shape_pdf = prop->shape()->pdf_uv(ray, intersection, transformation, area,
                                                  two_sided);

    float const material_pdf = material->emission_pdf(intersection.uv, filter, worker);

    return shape_pdf * material_pdf;
}

void Prop_image_light::prepare_sampling(uint32_t light_id, uint64_t time, Scene& scene,
                                        thread::Pool& pool) noexcept {
    //    scene.prop(prop_)->prepare_sampling(prop_, part_, light_id, time, true, pool, scene);
    scene.prop_prepare_sampling(prop_, part_, light_id, time, true, pool);
}

}  // namespace scene::light
