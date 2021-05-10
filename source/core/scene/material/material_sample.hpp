#ifndef SU_SCENE_MATERIAL_SAMPLE_HPP
#define SU_SCENE_MATERIAL_SAMPLE_HPP

#include "base/flags/flags.hpp"
#include "base/math/vector2.hpp"
#include "base/math/vector3.hpp"

namespace sampler {
class Sampler;
}

namespace rnd {
class Generator;
}

using RNG = rnd::Generator;

namespace scene {

struct Renderstate;
class Worker;

namespace material {

namespace bxdf {
struct Result;
struct Sample;
}  // namespace bxdf

struct Layer {
    Layer swapped() const;
    Layer swapped(bool same_side) const;

    void set_tangent_frame(float3_p t, float3_p b, float3_p n);
    void set_tangent_frame(float3_p n);

    float n_dot(float3_p v) const;

    float abs_n_dot(float3_p v) const;

    float clamp_n_dot(float3_p v) const;

    float clamp_abs_n_dot(float3_p v) const;

    float3 shading_normal() const;

    float3 tangent_to_world(float3_p v) const;

    float3 world_to_tangent(float3_p v) const;

    float3 t_;
    float3 b_;
    float3 n_;
};

class Sample {
  public:
    using Sampler = sampler::Sampler;

    Sample();

    virtual ~Sample();

    virtual bxdf::Result evaluate(float3_p wi) const = 0;

    virtual void sample(Sampler& sampler, RNG& rng, bxdf::Sample& result) const = 0;

    bool is_pure_emissive() const;
    bool is_translucent() const;
    bool can_evaluate() const;
    bool avoid_caustics() const;

    float3 geometric_normal() const;
    float3 interpolated_normal() const;
    float3 shading_normal() const;
    float3 shading_tangent() const;
    float3 shading_bitangent() const;

    float3 wo() const;

    float3 albedo() const;
    float3 radiance() const;

    float alpha() const;

    float clamp_geo_n_dot(float3_p v) const;

    bool same_hemisphere(float3_p v) const;

    void set_common(Renderstate const& rs, float3_p wo, float3_p albedo, float3_p radiance,
                    float2 alpha);

    void set_common(float3_p geo_n, float3_p n, float3_p wo, float alpha);

    void set_radiance(float3_p radiance);

    Layer layer_;

    // protected:
    float3 geo_n_;
    float3 n_;
    float3 wo_;
    float3 albedo_;
    float3 radiance_;

    float2 alpha_;

    enum class Property {
        None           = 0,
        Pure_emissive  = 1 << 0,
        Translucent    = 1 << 1,
        Can_evaluate   = 1 << 2,
        Avoid_caustics = 1 << 3
    };

    flags::Flags<Property> properties_;
};

struct IoR {
    IoR swapped() const;
    IoR swapped(bool same_side) const;

    float eta_t;
    float eta_i;
};

}  // namespace material
}  // namespace scene

#endif
