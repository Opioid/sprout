#include "light.hpp"
#include "base/math/aabb.inl"
#include "base/math/matrix4x4.inl"
#include "base/math/vector3.inl"
#include "sampler/sampler.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "scene/material/material.inl"
#include "scene/prop/prop.hpp"
#include "scene/scene.inl"
#include "scene/scene_worker.hpp"
#include "scene/scene_worker.inl"
#include "scene/shape/shape.inl"
#include "scene/shape/shape_intersection.hpp"
#include "scene/shape/shape_sample.hpp"

namespace scene::light {

using Filter      = material::Sampler_settings::Filter;
using Sample_to   = shape::Sample_to;
using Sample_from = shape::Sample_from;
using Sampler     = sampler::Sampler;

Light::Light() = default;

Light::Light(Type type, uint32_t prop, uint32_t part)
    : type_(type), prop_(prop), part_(part), extent_(0.f) {}

float Light::area() const {
    return extent_;
}

void Light::set_extent(float extent) {
    extent_ = extent;
}

static inline float3 prop_power(uint32_t prop, uint32_t part, float area, AABB const& scene_bb,
                                Scene const& scene) {
    float3 const radiance = scene.prop_material(prop, part)->average_radiance(area);

    if (scene.prop_shape(prop)->is_finite()) {
        return area * radiance;
    }

    return squared_length(scene_bb.extent()) * area * radiance;
}

static inline float3 volume_power(uint32_t prop, uint32_t part, float volume, Scene const& scene) {
    float3 const radiance = scene.prop_material(prop, part)->average_radiance(volume);

    return volume * radiance;
}

float3 Light::power(AABB const& scene_bb, Scene const& scene) const {
    switch (type_) {
        case Type::Null:
            return float3(0.f);
        case Type::Prop:
            return prop_power(prop_, part_, extent_, scene_bb, scene);
        case Type::Prop_image:
            return prop_power(prop_, part_, extent_, scene_bb, scene);
        case Type::Volume:
            return volume_power(prop_, part_, extent_, scene);
        case Type::Volume_image:
            return volume_power(prop_, part_, extent_, scene);
    }

    return float3(0.);
}

void Light::prepare_sampling(uint32_t light_id, uint64_t time, Scene& scene,
                             Threads& threads) const {
    switch (type_) {
        case Type::Null:
            break;
        case Type::Prop:
            scene.prop_prepare_sampling(prop_, part_, light_id, time, false, false, threads);
            break;
        case Type::Prop_image:
            scene.prop_prepare_sampling(prop_, part_, light_id, time, true, false, threads);
            break;
        case Type::Volume:
            scene.prop_prepare_sampling(prop_, part_, light_id, time, false, true, threads);
            break;
        case Type::Volume_image:
            scene.prop_prepare_sampling(prop_, part_, light_id, time, true, true, threads);
            break;
    }
}

static inline bool prop_sample(uint32_t prop, uint32_t part, float area, float3 const& p,
                               float3 const& n, Transformation const& transformation,
                               bool total_sphere, Sampler& sampler, uint32_t sampler_dimension,
                               Worker& worker, Sample_to& result) {
    shape::Shape const* shape = worker.scene().prop_shape(prop);

    auto const material = worker.scene().prop_material(prop, part);

    bool const two_sided = material->is_two_sided();

    if (total_sphere) {
        if (!shape->sample(part, p, transformation, area, two_sided, sampler, worker.rng(),
                           sampler_dimension, result)) {
            return false;
        }
    } else {
        if (!shape->sample(part, p, n, transformation, area, two_sided, sampler, worker.rng(),
                           sampler_dimension, result)) {
            return false;
        }

        if (dot(result.wi, n) <= 0.f) {
            return false;
        }
    }

    SOFT_ASSERT(result.pdf() > 0.f);

    return true;
}

static inline bool prop_image_sample(uint32_t prop, uint32_t part, float area, float3 const& p,
                                     float3 const& n, Transformation const& transformation,
                                     bool total_sphere, Sampler& sampler,
                                     uint32_t sampler_dimension, Worker& worker,
                                     Sample_to& result) {
    auto const material = worker.scene().prop_material(prop, part);

    float2 const s2d = sampler.generate_sample_2D(worker.rng(), sampler_dimension);

    auto const rs = material->radiance_sample(float3(s2d, 0.f));
    if (0.f == rs.pdf()) {
        return false;
    }

    bool const two_sided = material->is_two_sided();

    // this pdf includes the uv weight which adjusts for texture distortion by the shape
    if (!worker.scene().prop_shape(prop)->sample(part, p, rs.uvw.xy(), transformation, area,
                                                 two_sided, result)) {
        return false;
    }

    if (dot(result.wi, n) > 0.f || total_sphere) {
        result.pdf() *= rs.pdf();
        return true;
    }

    return false;
}

static inline bool volume_sample(uint32_t prop, uint32_t part, float volume, float3 const& p,
                                 float3 const& n, Transformation const& transformation,
                                 bool total_sphere, Sampler& sampler, uint32_t sampler_dimension,
                                 Worker& worker, Sample_to& result) {
    if (!worker.scene().prop_shape(prop)->sample_volume(part, p, transformation, volume, sampler,
                                                        worker.rng(), sampler_dimension, result)) {
        return false;
    }

    if (dot(result.wi, n) > 0.f || total_sphere) {
        return true;
    }

    return false;
}

static inline bool volume_image_sample(uint32_t prop, uint32_t part, float volume, float3 const& p,
                                       float3 const& n, Transformation const& transformation,
                                       bool total_sphere, Sampler& sampler,
                                       uint32_t sampler_dimension, Worker& worker,
                                       Sample_to& result) {
    auto const material = worker.scene().prop_material(prop, part);

    auto& rng = worker.rng();

    float2 const s2d = sampler.generate_sample_2D(rng, sampler_dimension);
    float const  s1d = sampler.generate_sample_1D(rng, sampler_dimension);

    auto const rs = material->radiance_sample(float3(s2d, s1d));
    if (0.f == rs.pdf()) {
        return false;
    }

    if (!worker.scene().prop_shape(prop)->sample(part, p, rs.uvw, transformation, volume, result)) {
        return false;
    }

    if (dot(result.wi, n) > 0.f || total_sphere) {
        result.pdf() *= rs.pdf();
        return true;
    }

    return false;
}

bool Light::sample(float3 const& p, float3 const& n, Transformation const& transformation,
                   bool total_sphere, Sampler& sampler, uint32_t sampler_dimension, Worker& worker,
                   Sample_to& result) const {
    switch (type_) {
        case Type::Null:
            return false;
        case Type::Prop:
            return prop_sample(prop_, part_, extent_, p, n, transformation, total_sphere, sampler,
                               sampler_dimension, worker, result);
        case Type::Prop_image:
            return prop_image_sample(prop_, part_, extent_, p, n, transformation, total_sphere,
                                     sampler, sampler_dimension, worker, result);
        case Type::Volume:
            return volume_sample(prop_, part_, extent_, p, n, transformation, total_sphere, sampler,
                                 sampler_dimension, worker, result);
        case Type::Volume_image:
            return volume_image_sample(prop_, part_, extent_, p, n, transformation, total_sphere,
                                       sampler, sampler_dimension, worker, result);
    }

    return false;
}

float3 Light::evaluate(Sample_to const& sample, Filter filter, Worker const& worker) const {
    if (Type::Null == type_) {
        return float3(0.f);
    }

    auto const material = worker.scene().prop_material(prop_, part_);

    return material->evaluate_radiance(sample.wi, sample.uvw, extent_, filter, worker);
}

static inline bool prop_sample(uint32_t prop, uint32_t part, float area,
                               Transformation const& transformation, Sampler& sampler,
                               uint32_t sampler_dimension, AABB const& bounds, Worker& worker,
                               Sample_from& result) {
    auto const material = worker.scene().prop_material(prop, part);

    bool const two_sided = material->is_two_sided();

    auto& rng = worker.rng();

    float2 const importance_uv = sampler.generate_sample_2D(rng);

    if (!worker.scene().prop_shape(prop)->sample(part, transformation, area, two_sided, sampler,
                                                 rng, sampler_dimension, importance_uv, bounds,
                                                 result)) {
        return false;
    }

    return true;
}

static inline bool prop_image_sample(uint32_t prop, uint32_t part, float area,
                                     Transformation const& transformation, Sampler& sampler,
                                     uint32_t sampler_dimension, AABB const& bounds, Worker& worker,
                                     Sample_from& result) {
    auto const material = worker.scene().prop_material(prop, part);

    auto& rng = worker.rng();

    float2 const s2d = sampler.generate_sample_2D(rng, sampler_dimension);

    auto const rs = material->radiance_sample(float3(s2d, 0.f));
    if (0.f == rs.pdf()) {
        return false;
    }

    bool const two_sided = material->is_two_sided();

    float2 const importance_uv = sampler.generate_sample_2D(rng);

    // this pdf includes the uv weight which adjusts for texture distortion by the shape
    if (!worker.scene().prop_shape(prop)->sample(part, rs.uvw.xy(), transformation, area, two_sided,
                                                 importance_uv, bounds, result)) {
        return false;
    }

    result.pdf *= rs.pdf();

    return true;
}

bool Light::sample(Transformation const& transformation, Sampler& sampler,
                   uint32_t sampler_dimension, AABB const& bounds, Worker& worker,
                   Sample_from& result) const {
    switch (type_) {
        case Type::Null:
            return false;
        case Type::Prop:
            return prop_sample(prop_, part_, extent_, transformation, sampler, sampler_dimension,
                               bounds, worker, result);
        case Type::Prop_image:
            return prop_image_sample(prop_, part_, extent_, transformation, sampler,
                                     sampler_dimension, bounds, worker, result);
        case Type::Volume:
            return false;
        case Type::Volume_image:
            return false;
    }

    return false;
}

static inline bool prop_sample(uint32_t prop, uint32_t part, float area,
                               Transformation const& transformation, Sampler& sampler,
                               uint32_t sampler_dimension, Distribution_2D const& importance,
                               AABB const& bounds, Worker& worker, Sample_from& result) {
    auto const material = worker.scene().prop_material(prop, part);

    bool const two_sided = material->is_two_sided();

    auto& rng = worker.rng();

    float2 const s2d = sampler.generate_sample_2D(rng);

    auto const importance_uv = importance.sample_continuous(s2d);
    if (0.f == importance_uv.pdf) {
        return false;
    }

    if (!worker.scene().prop_shape(prop)->sample(part, transformation, area, two_sided, sampler,
                                                 rng, sampler_dimension, importance_uv.uv, bounds,
                                                 result)) {
        return false;
    }

    result.pdf *= importance_uv.pdf;

    SOFT_ASSERT(result.pdf > 0.f);

    return true;
}

static inline bool prop_image_sample(uint32_t prop, uint32_t part, float area,
                                     Transformation const& transformation, Sampler& sampler,
                                     uint32_t sampler_dimension, Distribution_2D const& importance,
                                     AABB const& bounds, Worker& worker, Sample_from& result) {
    auto const material = worker.scene().prop_material(prop, part);

    auto& rng = worker.rng();

    float2 const s2d0 = sampler.generate_sample_2D(rng, sampler_dimension);

    auto const rs = material->radiance_sample(float3(s2d0, 0.f));
    if (0.f == rs.pdf()) {
        return false;
    }

    bool const two_sided = material->is_two_sided();

    float2 const s2d1 = sampler.generate_sample_2D(rng);

    auto const importance_uv = importance.sample_continuous(s2d1);
    if (0.f == importance_uv.pdf) {
        return false;
    }

    // this pdf includes the uv weight which adjusts for texture distortion by the shape
    if (!worker.scene().prop_shape(prop)->sample(part, rs.uvw.xy(), transformation, area, two_sided,
                                                 importance_uv.uv, bounds, result)) {
        return false;
    }

    result.pdf *= rs.pdf() * importance_uv.pdf;

    return true;
}

bool Light::sample(Transformation const& transformation, Sampler& sampler,
                   uint32_t sampler_dimension, Distribution_2D const& importance,
                   AABB const& bounds, Worker& worker, Sample_from& result) const {
    switch (type_) {
        case Type::Null:
            return false;
        case Type::Prop:
            return prop_sample(prop_, part_, extent_, transformation, sampler, sampler_dimension,
                               importance, bounds, worker, result);
        case Type::Prop_image:
            return prop_image_sample(prop_, part_, extent_, transformation, sampler,
                                     sampler_dimension, importance, bounds, worker, result);
        case Type::Volume:
            return false;
        case Type::Volume_image:
            return false;
    }

    return false;
}

float3 Light::evaluate(Sample_from const& sample, Filter filter, Worker const& worker) const {
    if (Type::Null == type_) {
        return float3(0.f);
    }

    auto const material = worker.scene().prop_material(prop_, part_);

    return material->evaluate_radiance(-sample.dir, float3(sample.uv, 0.f), extent_, filter,
                                       worker);
}

bool Light::sample(float3 const& p, float3 const& n, uint64_t time, bool total_sphere,
                   Sampler& sampler, uint32_t sampler_dimension, Worker& worker,
                   Sample_to& result) const {
    Transformation temp;
    auto const&    transformation = transformation_at(time, temp, worker.scene());

    return sample(p, n, transformation, total_sphere, sampler, sampler_dimension, worker, result);
}

bool Light::sample(float3 const& p, uint64_t time, Sampler& sampler, uint32_t sampler_dimension,
                   Worker& worker, Sample_to& result) const {
    Transformation temp;
    auto const&    transformation = transformation_at(time, temp, worker.scene());

    return sample(p, float3(0.f), transformation, true, sampler, sampler_dimension, worker, result);
}

bool Light::sample(uint64_t time, Sampler& sampler, uint32_t sampler_dimension, AABB const& bounds,
                   Worker& worker, Sample_from& result) const {
    Transformation temp;
    auto const&    transformation = transformation_at(time, temp, worker.scene());

    return sample(transformation, sampler, sampler_dimension, bounds, worker, result);
}

bool Light::sample(uint64_t time, Sampler& sampler, uint32_t sampler_dimension,
                   Distribution_2D const& importance, AABB const& bounds, Worker& worker,
                   Sample_from& result) const {
    Transformation temp;
    auto const&    transformation = transformation_at(time, temp, worker.scene());

    return sample(transformation, sampler, sampler_dimension, importance, bounds, worker, result);
}

static inline float prop_pdf(uint32_t prop, uint32_t part, float area, Ray const& ray,
                             shape::Intersection const& isec, Transformation const& transformation,
                             bool total_sphere, Worker const& worker) {
    bool const two_sided = worker.scene().prop_material(prop, part)->is_two_sided();

    return worker.scene().prop_shape(prop)->pdf(ray, isec, transformation, area, two_sided,
                                                total_sphere);
}

static inline float prop_image_pdf(uint32_t prop, uint32_t part, float area, Ray const& ray,
                                   shape::Intersection const& isec,
                                   Transformation const& transformation, Filter filter,
                                   Worker const& worker) {
    auto const material = worker.scene().prop_material(prop, part);

    bool const two_sided = material->is_two_sided();

    // this pdf includes the uv weight which adjusts for texture distortion by the shape
    float const shape_pdf = worker.scene().prop_shape(prop)->pdf_uv(ray, isec, transformation, area,
                                                                    two_sided);

    float const material_pdf = material->emission_pdf(float3(isec.uv, 0.f), filter, worker);

    return shape_pdf * material_pdf;
}

static float volume_pdf(uint32_t prop, uint32_t /*part*/, float volume, Ray const& ray,
                        shape::Intersection const& isec, Transformation const& transformation,
                        Worker const& worker) {
    return worker.scene().prop_shape(prop)->pdf_volume(ray, isec, transformation, volume);
}

static inline float volume_image_pdf(uint32_t prop, uint32_t part, float volume, Ray const& ray,
                                     shape::Intersection const& isec,
                                     Transformation const& transformation, Filter filter,
                                     Worker const& worker) {
    auto const material = worker.scene().prop_material(prop, part);

    float const shape_pdf = worker.scene().prop_shape(prop)->pdf_volume(ray, isec, transformation,
                                                                        volume);

    float const material_pdf = material->emission_pdf(isec.p, filter, worker);

    return shape_pdf * material_pdf;
}

float Light::pdf(Ray const& ray, Intersection const& isec, bool total_sphere, Filter filter,
                 Worker const& worker) const {
    Transformation temp;
    auto const&    transformation = transformation_at(ray.time, temp, worker.scene());

    switch (type_) {
        case Type::Null:
            return 0.f;
        case Type::Prop:
            return prop_pdf(prop_, part_, extent_, ray, isec, transformation, total_sphere, worker);
        case Type::Prop_image:
            return prop_image_pdf(prop_, part_, extent_, ray, isec, transformation, filter, worker);
        case Type::Volume:
            return volume_pdf(prop_, part_, extent_, ray, isec, transformation, worker);
        case Type::Volume_image:
            return volume_image_pdf(prop_, part_, extent_, ray, isec, transformation, filter,
                                    worker);
    }

    return 0.f;
}

}  // namespace scene::light
