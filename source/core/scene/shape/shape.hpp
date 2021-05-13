#ifndef SU_CORE_SCENE_SHAPE_SHAPE_HPP
#define SU_CORE_SCENE_SHAPE_SHAPE_HPP

#include "base/flags/flags.hpp"
#include "base/math/aabb.hpp"
#include "base/math/matrix.hpp"
#include "scene/material/sampler_settings.hpp"

namespace math {
struct Transformation;
}

namespace thread {
class Pool;
}

using Threads = thread::Pool;

namespace sampler {
class Sampler;
}

namespace rnd {
class Generator;
}

using RNG = rnd::Generator;

namespace scene {

namespace light {
class Tree_builder;
}

namespace material {
class Material;
}

struct Ray;
class Worker;

namespace entity {
struct Composed_transformation;
}

namespace shape {

struct Intersection;
struct Normals;
struct Sample_to;
struct Sample_from;
class Node_stack;
class Morphable;

class Shape {
  public:
    using Filter         = material::Sampler_settings::Filter;
    using Material       = material::Material;
    using Materials      = Material const* const*;
    using Sampler        = sampler::Sampler;
    using Transformation = entity::Composed_transformation;

    enum class Property { Complex = 1 << 0, Finite = 1 << 1, Analytical = 1 << 2 };

    using Properties = flags::Flags<Property>;

    static char const* identifier();

    Shape(Properties properties);

    virtual ~Shape();

    virtual AABB aabb() const = 0;

    virtual AABB part_aabb(uint32_t part, uint32_t variant) const;

    virtual uint32_t num_parts() const;

    virtual uint32_t num_materials() const;

    virtual uint32_t part_id_to_material_id(uint32_t part) const;

    virtual bool intersect(Ray& ray, Transformation const& trafo, Node_stack& nodes,
                           Intersection& isec) const = 0;

    virtual bool intersect_nsf(Ray& ray, Transformation const& trafo, Node_stack& nodes,
                               Intersection& isec) const = 0;

    virtual bool intersect(Ray& ray, Transformation const& trafo, Node_stack& nodes,
                           Normals& normals) const = 0;

    virtual bool intersect_p(Ray const& ray, Transformation const& trafo,
                             Node_stack& nodes) const = 0;

    virtual float visibility(Ray const& ray, Transformation const& trafo, uint32_t entity,
                             Filter filter, Worker& worker) const = 0;

    virtual bool thin_absorption(Ray const& ray, Transformation const& trafo, uint32_t entity,
                                 Filter filter, Worker& worker, float3& ta) const = 0;

    virtual bool sample(uint32_t part, uint32_t variant, float3_p p, float3_p n,
                        Transformation const& trafo, float area, bool two_sided, bool total_sphere,
                        Sampler& sampler, RNG& rng, uint32_t sampler_d,
                        Sample_to& sample) const = 0;

    virtual bool sample_volume(uint32_t part, float3_p p, Transformation const& trafo, float volume,
                               Sampler& sampler, RNG& rng, uint32_t sampler_d,
                               Sample_to& sample) const;

    virtual bool sample(uint32_t part, uint32_t variant, Transformation const& trafo, float area,
                        bool two_sided, Sampler& sampler, RNG& rng, uint32_t sampler_d,
                        float2 importance_uv, AABB const& bounds, Sample_from& sample) const = 0;

    // All pdf functions implicitely assume that the passed
    // ray/isec/trafo combination actually lead to a hit.
    virtual float pdf(uint32_t variant, Ray const& ray, float3_p n, Intersection const& isec,
                      Transformation const& trafo, float area, bool two_sided,
                      bool total_sphere) const = 0;

    virtual float pdf_volume(Ray const& ray, Intersection const& isec, Transformation const& trafo,
                             float volume) const = 0;

    // The following functions are used for textured lights
    // and should have the uv weight baked in!
    virtual bool sample(uint32_t part, float3_p p, float2 uv, Transformation const& trafo,
                        float area, bool two_sided, Sample_to& sample) const = 0;

    virtual bool sample_volume(uint32_t part, float3_p p, float3_p uvw, Transformation const& trafo,
                               float volume, Sample_to& sample) const = 0;

    virtual bool sample(uint32_t part, float2 uv, Transformation const& trafo, float area,
                        bool two_sided, float2 importance_uv, AABB const& bounds,
                        Sample_from& sample) const = 0;

    virtual float pdf_uv(Ray const& ray, Intersection const& isec, Transformation const& trafo,
                         float area, bool two_sided) const = 0;

    virtual float uv_weight(float2 uv) const = 0;
    // ---

    virtual float area(uint32_t part, float3_p scale) const = 0;

    virtual float volume(uint32_t part, float3_p scale) const = 0;

    struct Differential_surface {
        float3 dpdu;
        float3 dpdv;
    };
    virtual Differential_surface differential_surface(uint32_t primitive) const = 0;

    bool is_complex() const;

    bool is_finite() const;

    bool is_analytical() const;

    virtual uint32_t prepare_sampling(uint32_t part, uint32_t material,
                                      light::Tree_builder& builder, Worker const& worker,
                                      Threads& threads);

    virtual float4 cone(uint32_t part) const;

    virtual Morphable* morphable_shape();

  protected:
    static float constexpr Dot_min = 0.00001f;

  private:
    flags::Flags<Property> properties_;
};

}  // namespace shape
}  // namespace scene

#endif
