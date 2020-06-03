#ifndef SU_SCENE_LIGHT_LIGHT_INL
#define SU_SCENE_LIGHT_LIGHT_INL

#include "base/math/vector3.inl"
#include "light.hpp"
#include "sampler/sampler.hpp"
#include "scene/entity/composed_transformation.inl"
#include "scene/prop/prop.hpp"
#include "scene/prop/prop_intersection.hpp"
#include "scene/scene.hpp"
#include "scene/scene_ray.hpp"
#include "scene/scene_worker.hpp"
#include "scene/shape/shape.hpp"
#include "scene/shape/shape_sample.hpp"

namespace scene::light {

using Transformation = entity::Composed_transformation;
using Filter         = material::Sampler_settings::Filter;
using Sample_to      = shape::Sample_to;
using Sample_from    = shape::Sample_from;
using Sampler        = sampler::Sampler;

inline Light::Light(Type type, uint32_t prop, uint32_t part)
    : type_(type), prop_(prop), part_(part), extent_(0.f) {}

inline float Light::area() const {
    return extent_;
}

inline void Light::set_extent(float extent) {
    extent_ = extent;
}

inline Transformation const& Light::transformation_at(uint64_t time, Transformation& transformation,
                                                      Scene const& scene) const {
    return scene.prop_transformation_at(prop_, time, transformation);
}

inline bool Light::is_finite(Scene const& scene) const {
    return scene.prop_shape(prop_)->is_finite();
}

static inline bool prop_sample(uint32_t prop, uint32_t part, float area, float3 const& p,
                               float3 const& n, Transformation const& transformation,
                               bool total_sphere, Sampler& sampler, uint32_t sampler_dimension,
                               Worker const& worker, Sample_to& result) {
    shape::Shape const* shape = worker.scene().prop_shape(prop);

    auto const material = worker.scene().prop_material(prop, part);

    bool const two_sided = material->is_two_sided();

    if (total_sphere) {
        if (!shape->sample(part, p, transformation, area, two_sided, sampler, sampler_dimension,
                           result)) {
            return false;
        }
    } else {
        if (!shape->sample(part, p, n, transformation, area, two_sided, sampler, sampler_dimension,
                           result)) {
            return false;
        }

        if (dot(result.wi, n) <= 0.f) {
            return false;
        }
    }

    return true;
}

static inline bool prop_image_sample(uint32_t prop, uint32_t part, float area, float3 const& p,
                                     float3 const& n, Transformation const& transformation,
                                     bool total_sphere, Sampler& sampler,
                                     uint32_t sampler_dimension, Worker const& worker,
                                     Sample_to& result) {
    auto const material = worker.scene().prop_material(prop, part);

    float2 const s2d = sampler.generate_sample_2D(sampler_dimension);

    auto const rs = material->radiance_sample(s2d);
    if (0.f == rs.pdf) {
        return false;
    }

    bool const two_sided = material->is_two_sided();

    // this pdf includes the uv weight which adjusts for texture distortion by the shape
    if (!worker.scene().prop_shape(prop)->sample(part, p, rs.uv, transformation, area, two_sided,
                                                 result)) {
        return false;
    }

    if (dot(result.wi, n) > 0.f || total_sphere) {
        result.pdf() *= rs.pdf;
        return true;
    }

    return false;
}

static inline bool volume_sample(uint32_t prop, uint32_t part, float volume, float3 const& p,
                                 float3 const& n, Transformation const& transformation,
                                 bool total_sphere, Sampler& sampler, uint32_t sampler_dimension,
                                 Worker const& worker, Sample_to& result) {
    if (!worker.scene().prop_shape(prop)->sample_volume(part, p, transformation, volume, sampler,
                                                        sampler_dimension, result)) {
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
                                       uint32_t sampler_dimension, Worker const& worker,
                                       Sample_to& result) {
    auto const material = worker.scene().prop_material(prop, part);

    float2 const s2d = sampler.generate_sample_2D(sampler_dimension);
    float const  s1d = sampler.generate_sample_1D(sampler_dimension);

    auto const rs = material->radiance_sample(float3(s2d, s1d));
    if (0.f == rs.pdf) {
        return false;
    }

    if (!worker.scene().prop_shape(prop)->sample(part, p, rs.uvw, transformation, volume, result)) {
        return false;
    }

    if (dot(result.wi, n) > 0.f || total_sphere) {
        result.pdf() *= rs.pdf;
        return true;
    }

    return false;
}

inline bool Light::sample(float3 const& p, float3 const& n, Transformation const& transformation,
                          bool total_sphere, Sampler& sampler, uint32_t sampler_dimension,
                          Worker const& worker, Sample_to& result) const {
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

inline float3 Light::evaluate(Sample_to const& sample, Filter filter, Worker const& worker) const {
    if (Type::Null == type_) {
        return float3(0.f);
    }

    auto const material = worker.scene().prop_material(prop_, part_);

    return material->evaluate_radiance(sample.wi, sample.uvw, extent_, filter, worker);
}

static inline bool prop_sample(uint32_t prop, uint32_t part, float area,
                               Transformation const& transformation, Sampler& sampler,
                               uint32_t sampler_dimension, AABB const& bounds, Worker const& worker,
                               Sample_from& result) {
    auto const material = worker.scene().prop_material(prop, part);

    bool const two_sided = material->is_two_sided();

    float2 const importance_uv = sampler.generate_sample_2D();

    if (!worker.scene().prop_shape(prop)->sample(part, transformation, area, two_sided, sampler,
                                                 sampler_dimension, importance_uv, bounds,
                                                 result)) {
        return false;
    }

    return true;
}

static inline bool prop_image_sample(uint32_t prop, uint32_t part, float area,
                                     Transformation const& transformation, Sampler& sampler,
                                     uint32_t sampler_dimension, AABB const& bounds,
                                     Worker const& worker, Sample_from& result) {
    auto const material = worker.scene().prop_material(prop, part);

    float2 const s2d = sampler.generate_sample_2D(sampler_dimension);

    auto const rs = material->radiance_sample(s2d);
    if (0.f == rs.pdf) {
        return false;
    }

    bool const two_sided = material->is_two_sided();

    float2 const importance_uv = sampler.generate_sample_2D();

    // this pdf includes the uv weight which adjusts for texture distortion by the shape
    if (!worker.scene().prop_shape(prop)->sample(part, rs.uv, transformation, area, two_sided,
                                                 importance_uv, bounds, result)) {
        return false;
    }

    result.pdf *= rs.pdf;

    return true;
}

inline bool Light::sample(Transformation const& transformation, Sampler& sampler,
                          uint32_t sampler_dimension, AABB const& bounds, Worker const& worker,
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
                               AABB const& bounds, Worker const& worker, Sample_from& result) {
    auto const material = worker.scene().prop_material(prop, part);

    bool const two_sided = material->is_two_sided();

    float2 const s2d = sampler.generate_sample_2D();

    auto const importance_uv = importance.sample_continuous(s2d);
    if (0.f == importance_uv.pdf) {
        return false;
    }

    if (!worker.scene().prop_shape(prop)->sample(part, transformation, area, two_sided, sampler,
                                                 sampler_dimension, importance_uv.uv, bounds,
                                                 result)) {
        return false;
    }

    result.pdf *= importance_uv.pdf;

    return true;
}

static inline bool prop_image_sample(uint32_t prop, uint32_t part, float area,
                                     Transformation const& transformation, Sampler& sampler,
                                     uint32_t sampler_dimension, Distribution_2D const& importance,
                                     AABB const& bounds, Worker const& worker,
                                     Sample_from& result) {
    auto const material = worker.scene().prop_material(prop, part);

    float2 const s2d0 = sampler.generate_sample_2D(sampler_dimension);

    auto const rs = material->radiance_sample(s2d0);
    if (0.f == rs.pdf) {
        return false;
    }

    bool const two_sided = material->is_two_sided();

    float2 const s2d1 = sampler.generate_sample_2D();

    auto const importance_uv = importance.sample_continuous(s2d1);
    if (0.f == importance_uv.pdf) {
        return false;
    }

    // this pdf includes the uv weight which adjusts for texture distortion by the shape
    if (!worker.scene().prop_shape(prop)->sample(part, rs.uv, transformation, area, two_sided,
                                                 importance_uv.uv, bounds, result)) {
        return false;
    }

    result.pdf *= rs.pdf * importance_uv.pdf;

    return true;
}

inline bool Light::sample(Transformation const& transformation, Sampler& sampler,
                          uint32_t sampler_dimension, Distribution_2D const& importance,
                          AABB const& bounds, Worker const& worker, Sample_from& result) const {
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

inline float3 Light::evaluate(Sample_from const& sample, Filter filter,
                              Worker const& worker) const {
    if (Type::Null == type_) {
        return float3(0.f);
    }

    auto const material = worker.scene().prop_material(prop_, part_);

    return material->evaluate_radiance(-sample.dir, float3(sample.uv, 0.f), extent_, filter,
                                       worker);
}

inline bool Light::sample(float3 const& p, float3 const& n, uint64_t time, bool total_sphere,
                          Sampler& sampler, uint32_t sampler_dimension, Worker const& worker,
                          Sample_to& result) const {
    Transformation temp;
    auto const&    transformation = transformation_at(time, temp, worker.scene());

    return sample(p, n, transformation, total_sphere, sampler, sampler_dimension, worker, result);
}

inline bool Light::sample(float3 const& p, uint64_t time, Sampler& sampler,
                          uint32_t sampler_dimension, Worker const& worker,
                          Sample_to& result) const {
    Transformation temp;
    auto const&    transformation = transformation_at(time, temp, worker.scene());

    return sample(p, float3(0.f), transformation, true, sampler, sampler_dimension, worker, result);
}

inline bool Light::sample(uint64_t time, Sampler& sampler, uint32_t sampler_dimension,
                          AABB const& bounds, Worker const& worker, Sample_from& result) const {
    Transformation temp;
    auto const&    transformation = transformation_at(time, temp, worker.scene());

    return sample(transformation, sampler, sampler_dimension, bounds, worker, result);
}

inline bool Light::sample(uint64_t time, Sampler& sampler, uint32_t sampler_dimension,
                          Distribution_2D const& importance, AABB const& bounds,
                          Worker const& worker, Sample_from& result) const {
    Transformation temp;
    auto const&    transformation = transformation_at(time, temp, worker.scene());

    return sample(transformation, sampler, sampler_dimension, importance, bounds, worker, result);
}

static inline float prop_pdf(uint32_t prop, uint32_t part, float area, Ray const& ray,
                             shape::Intersection const& intersection,
                             Transformation const& transformation, bool total_sphere,
                             Worker const& worker) {
    bool const two_sided = worker.scene().prop_material(prop, part)->is_two_sided();

    return worker.scene().prop_shape(prop)->pdf(ray, intersection, transformation, area, two_sided,
                                                total_sphere);
}

static inline float prop_image_pdf(uint32_t prop, uint32_t part, float area, Ray const& ray,
                                   shape::Intersection const& intersection,
                                   Transformation const& transformation, Filter filter,
                                   Worker const& worker) {
    auto const material = worker.scene().prop_material(prop, part);

    bool const two_sided = material->is_two_sided();

    // this pdf includes the uv weight which adjusts for texture distortion by the shape
    float const shape_pdf = worker.scene().prop_shape(prop)->pdf_uv(
        ray, intersection, transformation, area, two_sided);

    float const material_pdf = material->emission_pdf(intersection.uv, filter, worker);

    return shape_pdf * material_pdf;
}

static float volume_pdf(uint32_t prop, uint32_t /*part*/, float volume, Ray const& ray,
                        shape::Intersection const& intersection,
                        Transformation const& transformation, Worker const& worker) {
    return worker.scene().prop_shape(prop)->pdf_volume(ray, intersection, transformation, volume);
}

static inline float volume_image_pdf(uint32_t prop, uint32_t part, float volume, Ray const& ray,
                                     shape::Intersection const& intersection,
                                     Transformation const& transformation, Filter filter,
                                     Worker const& worker) {
    auto const material = worker.scene().prop_material(prop, part);

    float const shape_pdf = worker.scene().prop_shape(prop)->pdf_volume(ray, intersection,
                                                                        transformation, volume);

    float const material_pdf = material->emission_pdf(intersection.p, filter, worker);

    return shape_pdf * material_pdf;
}

inline float Light::pdf(Ray const& ray, Intersection const& intersection, bool total_sphere,
                        Filter filter, Worker const& worker) const {
    Transformation temp;
    auto const&    transformation = transformation_at(ray.time, temp, worker.scene());

    switch (type_) {
        case Type::Null:
            return 0.f;
        case Type::Prop:
            return prop_pdf(prop_, part_, extent_, ray, intersection, transformation, total_sphere,
                            worker);
        case Type::Prop_image:
            return prop_image_pdf(prop_, part_, extent_, ray, intersection, transformation, filter,
                                  worker);
        case Type::Volume:
            return volume_pdf(prop_, part_, extent_, ray, intersection, transformation, worker);
        case Type::Volume_image:
            return volume_image_pdf(prop_, part_, extent_, ray, intersection, transformation,
                                    filter, worker);
    }

    return 0.f;
}

inline bool Light::equals(uint32_t prop, uint32_t part) const {
    return prop_ == prop && part_ == part;
}

inline bool Light::is_light(uint32_t id) {
    return Null != id;
}

inline bool Light::is_area_light(uint32_t id) {
    return 0 == (id & Volume_light_mask);
}

inline uint32_t Light::strip_mask(uint32_t id) {
    return ~Volume_light_mask & id;
}

}  // namespace scene::light

#endif
