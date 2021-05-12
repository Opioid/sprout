#include "light.hpp"
#include "base/math/aabb.inl"
#include "base/math/distribution_2d.hpp"
#include "base/math/matrix4x4.inl"
#include "base/math/vector3.inl"
#include "sampler/sampler.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "scene/material/material.inl"
#include "scene/prop/prop.hpp"
#include "scene/prop/prop_intersection.inl"
#include "scene/scene.inl"
#include "scene/scene_worker.hpp"
#include "scene/scene_worker.inl"
#include "scene/shape/shape.inl"
#include "scene/shape/shape_sample.hpp"

namespace scene::light {

using Filter       = material::Sampler_settings::Filter;
using Intersection = prop::Intersection;
using Sample_to    = shape::Sample_to;
using Sample_from  = shape::Sample_from;
using Sampler      = sampler::Sampler;

Light::Light() = default;

Light::Light(Type type, bool two_sided, uint32_t prop, uint32_t part)
    : type_(type), two_sided_(two_sided), prop_(prop), part_(part), variant_(0), extent_(0.f) {}

void Light::set_variant(uint32_t variant) {
    variant_ = variant;
}

void Light::set_extent(float extent) {
    extent_ = extent;
}

float3 Light::power(float3_p average_radiance, AABB const& scene_bb, Scene const& scene) const {
    float const extent = two_sided_ ? 2.f * extent_ : extent_;

    float3 const radiance = extent * average_radiance;

    if (scene.prop_shape(prop_)->is_finite()) {
        return radiance;
    }

    return squared_length(scene_bb.extent()) * radiance;
}

void Light::prepare_sampling(uint32_t light_id, uint64_t time, Scene& scene, Worker& worker,
                             Threads& threads) const {
    switch (type_) {
        case Type::Prop:
        case Type::Prop_image:
            scene.prop_prepare_sampling(prop_, part_, light_id, time, false, worker, threads);
            break;
        case Type::Volume:
        case Type::Volume_image:
            scene.prop_prepare_sampling(prop_, part_, light_id, time, true, worker, threads);
            break;
    }
}

static inline bool prop_sample(uint32_t prop, uint32_t part, uint32_t variant, float area,
                               float3_p p, float3_p n, Transformation const& trafo, bool two_sided,
                               bool total_sphere, Sampler& sampler, uint32_t sampler_d,
                               Worker& worker, Sample_to& result) {
    shape::Shape const* shape = worker.scene().prop_shape(prop);

    if (!shape->sample(part, variant, p, n, trafo, area, two_sided, total_sphere, sampler,
                       worker.rng(), sampler_d, result)) {
        return false;
    }

    return (dot(result.wi, n) > 0.f) | total_sphere;
}

static inline bool prop_image_sample(uint32_t prop, uint32_t part, float area, float3_p p,
                                     float3_p n, Transformation const& trafo, bool two_sided,
                                     bool total_sphere, Sampler& sampler, uint32_t sampler_d,
                                     Worker& worker, Sample_to& result) {
    auto const material = worker.scene().prop_material(prop, part);

    float2 const s2d = sampler.sample_2D(worker.rng(), sampler_d);

    auto const rs = material->radiance_sample(float3(s2d, 0.f));
    if (0.f == rs.pdf()) {
        return false;
    }

    // this pdf includes the uv weight which adjusts for texture distortion by the shape
    if (!worker.scene().prop_shape(prop)->sample(part, p, rs.uvw.xy(), trafo, area, two_sided,
                                                 result)) {
        return false;
    }

    result.pdf() *= rs.pdf();

    return (dot(result.wi, n) > 0.f) | total_sphere;
}

static inline bool volume_sample(uint32_t prop, uint32_t part, float volume, float3_p p, float3_p n,
                                 Transformation const& trafo, bool total_sphere, Sampler& sampler,
                                 uint32_t sampler_d, Worker& worker, Sample_to& result) {
    if (!worker.scene().prop_shape(prop)->sample_volume(part, p, trafo, volume, sampler,
                                                        worker.rng(), sampler_d, result)) {
        return false;
    }

    return (dot(result.wi, n) > 0.f) | total_sphere;
}

static inline bool volume_image_sample(uint32_t prop, uint32_t part, float volume, float3_p p,
                                       float3_p n, Transformation const& trafo, bool total_sphere,
                                       Sampler& sampler, uint32_t sampler_d, Worker& worker,
                                       Sample_to& result) {
    auto const material = worker.scene().prop_material(prop, part);

    auto& rng = worker.rng();

    float2 const s2d = sampler.sample_2D(rng, sampler_d);
    float const  s1d = sampler.sample_1D(rng, sampler_d);

    auto const rs = material->radiance_sample(float3(s2d, s1d));
    if (0.f == rs.pdf()) {
        return false;
    }

    if (!worker.scene().prop_shape(prop)->sample_volume(part, p, rs.uvw, trafo, volume, result)) {
        return false;
    }

    result.pdf() *= rs.pdf();

    return (dot(result.wi, n) > 0.f) | total_sphere;
}

bool Light::sample(float3_p p, float3_p n, uint64_t time, bool total_sphere, Sampler& sampler,
                   uint32_t sampler_d, Worker& worker, Sample_to& result) const {
    Transformation temp;
    auto const&    trafo = transformation_at(time, temp, worker.scene());

    switch (type_) {
        case Type::Prop:
            return prop_sample(prop_, part_, variant_, extent_, p, n, trafo, two_sided_,
                               total_sphere, sampler, sampler_d, worker, result);
        case Type::Prop_image:
            return prop_image_sample(prop_, part_, extent_, p, n, trafo, two_sided_, total_sphere,
                                     sampler, sampler_d, worker, result);
        case Type::Volume:
            return volume_sample(prop_, part_, extent_, p, n, trafo, total_sphere, sampler,
                                 sampler_d, worker, result);
        case Type::Volume_image:
            return volume_image_sample(prop_, part_, extent_, p, n, trafo, total_sphere, sampler,
                                       sampler_d, worker, result);
    }

    return false;
}

float3 Light::evaluate(Sample_to const& sample, Filter filter, Worker const& worker) const {
    auto const material = worker.scene().prop_material(prop_, part_);

    return material->evaluate_radiance(sample.wi, sample.n, sample.uvw, extent_, filter, worker);
}

static inline bool prop_sample(uint32_t prop, uint32_t part, float area,
                               Transformation const& trafo, Sampler& sampler, uint32_t sampler_d,
                               AABB const& bounds, Worker& worker, Sample_from& result) {
    auto const material = worker.scene().prop_material(prop, part);

    bool const two_sided = material->is_two_sided();

    auto& rng = worker.rng();

    float2 const importance_uv = sampler.sample_2D(rng);

    if (!worker.scene().prop_shape(prop)->sample(part, trafo, area, two_sided, sampler, rng,
                                                 sampler_d, importance_uv, bounds, result)) {
        return false;
    }

    return true;
}

static inline bool prop_image_sample(uint32_t prop, uint32_t part, float area,
                                     Transformation const& trafo, Sampler& sampler,
                                     uint32_t sampler_d, AABB const& bounds, Worker& worker,
                                     Sample_from& result) {
    auto const material = worker.scene().prop_material(prop, part);

    auto& rng = worker.rng();

    float2 const s2d = sampler.sample_2D(rng, sampler_d);

    auto const rs = material->radiance_sample(float3(s2d, 0.f));
    if (0.f == rs.pdf()) {
        return false;
    }

    bool const two_sided = material->is_two_sided();

    float2 const importance_uv = sampler.sample_2D(rng);

    // this pdf includes the uv weight which adjusts for texture distortion by the shape
    if (!worker.scene().prop_shape(prop)->sample(part, rs.uvw.xy(), trafo, area, two_sided,
                                                 importance_uv, bounds, result)) {
        return false;
    }

    result.pdf() *= rs.pdf();

    return true;
}

bool Light::sample(uint64_t time, Sampler& sampler, uint32_t sampler_d, AABB const& bounds,
                   Worker& worker, Sample_from& result) const {
    Transformation temp;
    auto const&    trafo = transformation_at(time, temp, worker.scene());

    switch (type_) {
        case Type::Prop:
            return prop_sample(prop_, part_, extent_, trafo, sampler, sampler_d, bounds, worker,
                               result);
        case Type::Prop_image:
            return prop_image_sample(prop_, part_, extent_, trafo, sampler, sampler_d, bounds,
                                     worker, result);
        case Type::Volume:
            return false;
        case Type::Volume_image:
            return false;
    }

    return false;
}

static inline bool prop_sample(uint32_t prop, uint32_t part, float area,
                               Transformation const& trafo, Sampler& sampler, uint32_t sampler_d,
                               Distribution_2D const& importance, AABB const& bounds,
                               Worker& worker, Sample_from& result) {
    auto const material = worker.scene().prop_material(prop, part);

    bool const two_sided = material->is_two_sided();

    auto& rng = worker.rng();

    float2 const s2d = sampler.sample_2D(rng);

    auto const importance_uv = importance.sample_continuous(s2d);
    if (0.f == importance_uv.pdf) {
        return false;
    }

    if (!worker.scene().prop_shape(prop)->sample(part, trafo, area, two_sided, sampler, rng,
                                                 sampler_d, importance_uv.uv, bounds, result)) {
        return false;
    }

    result.pdf() *= importance_uv.pdf;

    SOFT_ASSERT(result.pdf() > 0.f);

    return true;
}

static inline bool prop_image_sample(uint32_t prop, uint32_t part, float area,
                                     Transformation const& trafo, Sampler& sampler,
                                     uint32_t sampler_d, Distribution_2D const& importance,
                                     AABB const& bounds, Worker& worker, Sample_from& result) {
    auto const material = worker.scene().prop_material(prop, part);

    auto& rng = worker.rng();

    float2 const s2d0 = sampler.sample_2D(rng, sampler_d);

    auto const rs = material->radiance_sample(float3(s2d0, 0.f));
    if (0.f == rs.pdf()) {
        return false;
    }

    bool const two_sided = material->is_two_sided();

    float2 const s2d1 = sampler.sample_2D(rng);

    auto const importance_uv = importance.sample_continuous(s2d1);
    if (0.f == importance_uv.pdf) {
        return false;
    }

    // this pdf includes the uv weight which adjusts for texture distortion by the shape
    if (!worker.scene().prop_shape(prop)->sample(part, rs.uvw.xy(), trafo, area, two_sided,
                                                 importance_uv.uv, bounds, result)) {
        return false;
    }

    result.pdf() *= rs.pdf() * importance_uv.pdf;

    return true;
}

bool Light::sample(uint64_t time, Sampler& sampler, uint32_t sampler_d,
                   Distribution_2D const& importance, AABB const& bounds, Worker& worker,
                   Sample_from& result) const {
    Transformation temp;
    auto const&    trafo = transformation_at(time, temp, worker.scene());

    switch (type_) {
        case Type::Prop:
            return prop_sample(prop_, part_, extent_, trafo, sampler, sampler_d, importance, bounds,
                               worker, result);
        case Type::Prop_image:
            return prop_image_sample(prop_, part_, extent_, trafo, sampler, sampler_d, importance,
                                     bounds, worker, result);
        case Type::Volume:
            return false;
        case Type::Volume_image:
            return false;
    }

    return false;
}

float3 Light::evaluate(Sample_from const& sample, Filter filter, Worker const& worker) const {
    auto const material = worker.scene().prop_material(prop_, part_);

    return material->evaluate_radiance(-sample.dir, sample.n, float3(sample.uv, 0.f), extent_,
                                       filter, worker);
}

static inline float prop_pdf(float area, Ray const& ray, float3_p n, Intersection const& isec,
                             Transformation const& trafo, bool total_sphere, Worker const& worker) {
    bool const two_sided = isec.material(worker)->is_two_sided();

    return isec.shape(worker)->pdf(ray, n, isec.geo, trafo, area, two_sided, total_sphere);
}

static inline float prop_image_pdf(float area, Ray const& ray, Intersection const& isec,
                                   Transformation const& trafo, Worker const& worker) {
    auto const material = isec.material(worker);

    bool const two_sided = material->is_two_sided();

    float const material_pdf = material->emission_pdf(float3(isec.geo.uv, 0.f), worker);

    // this pdf includes the uv weight which adjusts for texture distortion by the shape
    float const shape_pdf = isec.shape(worker)->pdf_uv(ray, isec.geo, trafo, area, two_sided);

    return shape_pdf * material_pdf;
}

static float volume_pdf(float volume, Ray const& ray, Intersection const& isec,
                        Transformation const& trafo, Worker const& worker) {
    return isec.shape(worker)->pdf_volume(ray, isec.geo, trafo, volume);
}

static inline float volume_image_pdf(float volume, Ray const& ray, Intersection const& isec,
                                     Transformation const& trafo, Worker const& worker) {
    float const shape_pdf = isec.shape(worker)->pdf_volume(ray, isec.geo, trafo, volume);

    float const material_pdf = isec.material(worker)->emission_pdf(isec.geo.p, worker);

    return shape_pdf * material_pdf;
}

float Light::pdf(Ray const& ray, float3_p n, Intersection const& isec, bool total_sphere,
                 Worker const& worker) const {
    SOFT_ASSERT(isec.prop == prop_ && isec.geo.part == part_);

    Transformation temp;
    auto const&    trafo = transformation_at(ray.time, temp, worker.scene());

    switch (type_) {
        case Type::Prop:
            return prop_pdf(extent_, ray, n, isec, trafo, total_sphere, worker);
        case Type::Prop_image:
            return prop_image_pdf(extent_, ray, isec, trafo, worker);
        case Type::Volume:
            return volume_pdf(extent_, ray, isec, trafo, worker);
        case Type::Volume_image:
            return volume_image_pdf(extent_, ray, isec, trafo, worker);
    }

    return 0.f;
}

}  // namespace scene::light
